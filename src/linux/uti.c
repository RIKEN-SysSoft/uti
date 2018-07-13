/**
 * \file uti.c
 *  License details are found in the file LICENSE.
 * \brief
 *  Bridge UTI API to UTI system calls
 * \author Masamichi Takagi <masamichi.takagi@riken.jp> \par
 *      Copyright (C) 2016-2017 RIKEN AICS
 * \author Tomoki Shirasawa <tomoki.shirasawa.kk@hitachi-solutions.com> \par
 *      Copyright (C) 2017 Hitachi, Ltd.
 */

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <errno.h>
#include <sched.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/capability.h>
#include <hwloc.h>
#include "uti.h"
#include "uti_impl.h"

int *nthr_per_pu;

static hwloc_topology_t topo;

#define BITS_PER_ENTRY (sizeof(uint64_t) * 8)
#define NODE_ISSET(i, bitmap) !!((bitmap[i / BITS_PER_ENTRY] >> (i % BITS_PER_ENTRY)) & 1)

static int find_next_one(int i, uint64_t *bitmap) {
	while (i < UTI_MAX_NUMA_DOMAINS && !NODE_ISSET(i, bitmap)) {
		i++;
	}
	return i;
}

#define for_each_node(i, bitmap) for (i = 0; i < UTI_MAX_NUMA_DOMAINS; i = find_next_one(i, bitmap))

int uti_attr_init(uti_attr_t *attr)
{
	memset(attr, 0, sizeof(uti_attr_t));
	return 0;
}

int uti_attr_destroy(uti_attr_t *attr)
{
	return 0;
}

__attribute__((constructor)) void uti_init(void)
{
	int ret;

	/* Discover topology */
	if ((ret = hwloc_topology_init(&topo))) {
		pr_err("%s: error: hwloc_topology_init\n",
			__func__);
		return;
	}

	if ((ret = hwloc_topology_load(topo))) {
		pr_err("%s: error: hwloc_topology_load\n",
			__func__);
		return;
	}

	nthr_per_pu = (unsigned int *)calloc(hwloc_get_nbobjs_by_type(topo, HWLOC_OBJ_PU), sizeof(unsigned int));

	if (!nthr_per_pu) {
		pr_err("%s: error: allocating nthr_per_pu\n",
		       __func__);
		return;
	}
}

__attribute__((destructor)) void uti_finalize()
{
	hwloc_topology_destroy(topo);
}

static int uti_string_to_glibc_sched_affinity(char *_cpu_list, cpu_set_t *schedset)
{
	int ret = 0;
	int i;
	char *cpu_list;
	char *token, *minus;

	if (!(cpu_list = strdup(_cpu_list))) {
		pr_err("%s: error: allocating cpu_list\n",
			__func__);
		ret = -errno;
		goto out;
	}

	token = strsep(&cpu_list, ",");
	while (token) {
		if (*token == 0) {
			pr_err("%s: error: illegal expression: %s\n",
				__func__, _cpu_list); /* empty token */
			ret = -EINVAL;
			goto out;
		}
		if ((minus = strchr(token, '-'))) {
			int start, end;

			if (*(minus + 1) == 0) {
				pr_err("%s: error: illegal expression: %s\n",
					__func__, _cpu_list); /* empty token */
				ret = -EINVAL;
				goto out;
			}
			*minus = 0;
			start = atoi(token);
			end = atoi(minus + 1);
			for (i = start; i <= end; i++) {
				pr_debug("%s: schedset[%d]=1\n", __func__, i);
				CPU_SET(i, schedset);
			}
		} else {
			pr_debug("%s: schedset[%d]=1\n",
				__func__, atoi(token));
			CPU_SET(atoi(token), schedset);
		}
		token = strsep(&cpu_list, ",");
	}

	ret = 0;
 out:
	free(cpu_list);
	return ret;
}

static int uti_rr_allocate_cpu(hwloc_cpuset_t cpuset)
{
	int i;
	int mincpu;
	unsigned int minrr;
	unsigned int newval;
	unsigned int oldval;

retry:
	minrr = (unsigned int)-1;
	mincpu = -1;
	hwloc_bitmap_foreach_begin(i, cpuset) {
		int rr = nthr_per_pu[i];
		if (rr < minrr) {
			mincpu = i;
			minrr = rr;
		}
	} hwloc_bitmap_foreach_end();

	newval = minrr + 1;
	oldval = __sync_val_compare_and_swap(nthr_per_pu + mincpu, minrr, newval);
	if (oldval != minrr) {
		goto retry;
	}

	return mincpu;
}

static int uti_set_pthread_attr(pthread_attr_t *pthread_attr, uti_attr_t* uti_attr)
{
	int ret;
	int i;
	int parent_cpu_os_index;
	hwloc_obj_t parent_cpu, parent_numanode, parent_l1, parent_l2, parent_l3;
	hwloc_cpuset_t cpuset, tmpset;
	char *envset_str;
	cap_t cap = NULL;

	/* Find subgroups to which the caller belongs */
	parent_cpu_os_index = sched_getcpu();
	parent_cpu = hwloc_get_pu_obj_by_os_index(topo, parent_cpu_os_index);

	if (!parent_cpu) {
                pr_err("%s: error: cpu with os_index of %d not found\n",
			__func__, parent_cpu_os_index);
		ret = -ENOENT;
                goto out;
	}

	parent_numanode = hwloc_get_ancestor_obj_by_type(topo, HWLOC_OBJ_NUMANODE, parent_cpu);
	parent_l1 = hwloc_get_ancestor_obj_by_type(topo, HWLOC_OBJ_L1CACHE, parent_cpu);
	parent_l2 = hwloc_get_ancestor_obj_by_type(topo, HWLOC_OBJ_L2CACHE, parent_cpu);
	parent_l3 = hwloc_get_ancestor_obj_by_type(topo, HWLOC_OBJ_L3CACHE, parent_cpu);
	pr_debug("parent_numanode->os_index=%d\n", parent_numanode->os_index);

	/* Set initial cpuset to allowed & UTI_CPU_SET */
	cpuset = hwloc_bitmap_dup(hwloc_topology_get_allowed_cpuset(topo));

	if (!(tmpset = hwloc_bitmap_alloc())) {
                pr_err("%s: error: hwloc_bitmap_alloc\n",
			__func__);
		ret = -ENOMEM;
		goto out;
	}

	envset_str = getenv("UTI_CPU_SET");
	if (envset_str) {
		cpu_set_t envset;
		CPU_ZERO(envset);
		if (!(ret = uti_string_to_glibc_sched_affinity(envset_str, &envset))) {
			hwloc_cpuset_from_glibc_sched_affinity(topo, tmpset, &envset);
			hwloc_bitmap_and(cpuset, cpuset, tmpset);
		} else {
			pr_warn("%s: warning: uti_string_to_glibc_sched_affinity returned %d\n",
				__func__, ret);
		}
	}

	/* Narrow down cpuset */
	if (uti_attr->flags & UTI_FLAG_NUMA_SET) {
		hwloc_bitmap_zero(tmpset);
		for_each_node(i, uti_attr->numa_set) {
			hwloc_obj_t numanode = hwloc_get_numanode_obj_by_os_index(topo, i);
			hwloc_bitmap_or(tmpset, tmpset, numanode->cpuset);
		}
		hwloc_bitmap_and(cpuset, cpuset, tmpset);
	}

	if (uti_attr->flags & UTI_FLAG_SAME_NUMA_DOMAIN) {
		hwloc_bitmap_and(cpuset, cpuset, parent_numanode->cpuset);
	}

	if (uti_attr->flags & UTI_FLAG_DIFFERENT_NUMA_DOMAIN) {
		hwloc_bitmap_andnot(cpuset, cpuset, parent_numanode->cpuset);
	}

	if (uti_attr->flags & UTI_FLAG_SAME_L1) {
		hwloc_bitmap_and(cpuset, cpuset, parent_l1->cpuset);
	}

	if (uti_attr->flags & UTI_FLAG_DIFFERENT_L1) {
		hwloc_bitmap_andnot(cpuset, cpuset, parent_l1->cpuset);
	}

	if ((uti_attr->flags & UTI_FLAG_SAME_L2) && parent_l2) {
		hwloc_bitmap_and(cpuset, cpuset, parent_l2->cpuset);
	}

	if ((uti_attr->flags & UTI_FLAG_DIFFERENT_L2) && parent_l2) {
		hwloc_bitmap_andnot(cpuset, cpuset, parent_l2->cpuset);
	}

	if ((uti_attr->flags & UTI_FLAG_SAME_L3) && parent_l3) {
		hwloc_bitmap_and(cpuset, cpuset, parent_l3->cpuset);
	}

	if ((uti_attr->flags & UTI_FLAG_DIFFERENT_L3) && parent_l3) {
		hwloc_bitmap_andnot(cpuset, cpuset, parent_l3->cpuset);
	}

	if (uti_attr->flags &
	    (UTI_FLAG_EXCLUSIVE_CPU | UTI_FLAG_CPU_INTENSIVE)) {
		hwloc_bitmap_only(cpuset, uti_rr_allocate_cpu(cpuset));
	}

	/* Write cpuset to pthread_attr_t */
	ret = hwloc_bitmap_weight(cpuset);
	if (ret > 0) {
		cpu_set_t schedset;
		hwloc_cpuset_to_glibc_sched_affinity(topo, cpuset, &schedset, sizeof(schedset));
		if ((ret = pthread_setaffinity_np(pthread_attr, sizeof(cpu_set_t), &schedset))) {
			pr_err("%s: error: pthread_setaffinity_np\n",
				__func__);
			goto out;
		}
	} else {
		pr_warn("%s: warning: cpuset is empty\n", __func__);
	}

	/* Use real-time scheduler */
	if (uti_attr->flags & UTI_FLAG_HIGH_PRIORITY) {
		struct rlimit rlimit;
		cap_flag_value_t cap_flag_value;
	
		if (!(cap = cap_get_proc())) {
			pr_err("%s: error: cap_get_proc\n",
				__func__);
			goto out;
		}

		if ((cap_get_flag(cap, CAP_SYS_NICE, CAP_EFFECTIVE, &cap_flag_value))) {
			pr_err("%s: error: cap_get_flag\n",
				__func__);
			goto out;

		}

		if ((getrlimit(RLIMIT_RTPRIO, &rlimit))) {
			pr_err("%s: error: getrilimit\n",
				__func__);
			goto out;
		}

		/* Process needs CAP_SYS_NICE or positive RLIMIT_RTPRIO */
		if (cap_flag_value == CAP_SET || rlimit.rlim_cur > 0) {
			struct sched_param param = { .sched_priority = 99 };

			if ((ret = pthread_attr_setinheritsched(pthread_attr, PTHREAD_EXPLICIT_SCHED))) {
				pr_err("%s: error: pthread_attr_setinheritsched\n",
				       __func__);
				goto out;
			}

			if ((ret = pthread_attr_setschedpolicy(pthread_attr, SCHED_FIFO))) {
				pr_err("%s: error: pthread_attr_setschedpolicy\n",
				       __func__);
				goto out;
			}
			
			if ((ret = pthread_attr_setschedparam(pthread_attr, &param))) {
				pr_err("%s: error: pthread_setschedparam\n",
				       __func__);
				goto out;
			}
		} else {
			pr_warn("%s: warning: not enough capability/rlimit\n",
			       __func__);
		}
	}

	ret = 0;
out:
	cap_free(cap);
	hwloc_bitmap_free(cpuset);
	hwloc_bitmap_free(tmpset);
	return ret;
}

int uti_pthread_create(pthread_t *thread, pthread_attr_t *pthread_attr,
                       void *(*start_routine) (void *), void *arg, uti_attr_t *uti_attr)
{
	int ret;
	char *disable_uti_str;
	int disable_uti;
	
	disable_uti_str = getenv("DISABLE_UTI");
	disable_uti = disable_uti_str ? atoi(disable_uti_str) : 0;

	if (!disable_uti) {
		if ((ret = uti_set_pthread_attr(pthread_attr, uti_attr))) {
			pr_err("%s: error: uti_set_pthread_attr\n",
			       __func__);
			goto out;
		}
	}

	if ((ret = pthread_create(thread, pthread_attr, start_routine, arg))) {
		pr_err("%s: error: pthread_create: %s\n",
		       __func__, strerror(ret));
		goto out;
	}

	ret = 0;
out:
	return ret;
}
