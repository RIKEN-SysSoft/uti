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

#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <stdbool.h>
#include <errno.h>
#include <sched.h>
#include <pthread.h>
#include <math.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/capability.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <hwloc.h>
#include <hwloc/glibc-sched.h>
#include <hwloc/openfabrics-verbs.h>
#include "uti.h"
#include "uti_impl.h"

#define N_SHM_RETRY 10000 /* 10 sec */

static hwloc_topology_t topo;
static int max_cpu_os_index = -1;

static int shm_fd;
static char shm_fn[] = "/uti";
static int shm_leader;
static int *nthr_per_pu; /* #threads histogram indexed by hwloc_obj::os_index */

/* TODO: Construct this list at run-time by using ibv_get_device_name() */
struct uti_fabric uti_fabrics[] = {
	{ .name = "mlx5_0" },
	{ .name = "hfi1_0" },
	{ .name = NULL }
};


#define BITS_PER_ENTRY (sizeof(uint64_t) * 8)
#define NODE_ISSET(i, bitmap) !!((bitmap[i / BITS_PER_ENTRY] >> (i % BITS_PER_ENTRY)) & 1)


#define SIZE_REMAINING(start, cur, max) (max - (cur - start))
#define BITSOF_PRINT_UNIT (32)

static void glibc_sched_setaffinity_snprintf(char *buf, size_t size, cpu_set_t *schedset)
{
	int i, j, k, nwords;
	uint32_t word = 0;
	char *cur = buf;
	int rem = (max_cpu_os_index + 1) % BITSOF_PRINT_UNIT;

	for (nwords = 0, i = max_cpu_os_index; i >= 0; nwords++) {
		for (j = 0, word = 0;
		     j < ((nwords == 0 && rem != 0) ? rem : BITSOF_PRINT_UNIT);
		     j++, i--) {

			if (CPU_ISSET(i, schedset)) {
				word |= 1ULL << (i % BITSOF_PRINT_UNIT);
			}
		}
		if (nwords > 0) {
			cur += snprintf(cur, SIZE_REMAINING(buf, cur, size), " ");
		}
		
		if (nwords == 0 && rem != 0) {
			for (k = ceil(rem / 4.0) - 1; k >= 0; k--) {
				cur += snprintf(cur, SIZE_REMAINING(buf, cur, size), "%1x", word >> (k * 4) & 0xf);
			}
		} else {
			cur += snprintf(cur, SIZE_REMAINING(buf, cur, size), "%08x", word);
		}
	}
}

/* 1 character per 4 bits, 1 space per 32 bits, ..., \0 at the end */
#define MAX_SCHEDSET_STR \
(ceil((max_cpu_os_index + 1) / 4.0) +\
 (max_cpu_os_index + 1) / 32 +\
 1)
#define schedset_alloc() ((char *)malloc(MAX_SCHEDSET_STR))

static void schedset_snprintf(char *dst, size_t size, cpu_set_t *schedset)
{
	//pr_debug("%s: info: MAX_SCHEDSET_STR: %f\n", __func__, MAX_SCHEDSET_STR);
	glibc_sched_setaffinity_snprintf(dst, size, schedset);
}

static void pr_schedset(char *msg, cpu_set_t *schedset)
{
	char *buf;
	if (!(buf = schedset_alloc())) {
		pr_err("pr_debug_cpuset: error: allocating buf\n");
	} else {
		schedset_snprintf(buf, MAX_SCHEDSET_STR, schedset);
		pr_debug("%s: %s\n", msg, buf);
	}
	free(buf);
}

static void cpuset_snprintf(char *dst, size_t size, hwloc_cpuset_t cpuset)
{
		cpu_set_t schedset;
		hwloc_cpuset_to_glibc_sched_affinity(topo, cpuset, &schedset, sizeof(schedset));
		schedset_snprintf(dst, size, &schedset);
}

static void pr_cpuset(char *msg, hwloc_cpuset_t cpuset)
{
	char *buf;
	if (!(buf = schedset_alloc())) {
		pr_err("pr_debug_cpuset: error: allocating buf\n");
	} else {
		cpuset_snprintf(buf, MAX_SCHEDSET_STR, cpuset);
		pr_debug("%s: %s\n", msg, buf);
	}
	free(buf);
}

static int find_next_one(int i, uint64_t *bitmap) {
	int j;
	for (j = i + 1; j < UTI_MAX_NUMA_DOMAINS && !NODE_ISSET(j, bitmap); j++) {
	}
	return j;
}

#define for_each_node(i, bitmap) for (i = find_next_one(-1, bitmap); i < UTI_MAX_NUMA_DOMAINS; i = find_next_one(i, bitmap))


hwloc_obj_t get_cache_obj_by_pu(hwloc_obj_type_t type, hwloc_obj_t cpu)
{
	hwloc_obj_t obj = NULL;
	while ((obj = hwloc_get_next_obj_by_type(topo, type, obj))) {
		//pr_cpuset("get_cache_obj_by_pu", obj->cpuset);
		if (hwloc_bitmap_isset(obj->cpuset, cpu->os_index)) {
			break;
		}
	}
	return obj;
}

hwloc_obj_t get_numanode_obj_by_pu(hwloc_obj_t cpu)
{
	hwloc_obj_t obj = NULL;
	while ((obj = hwloc_get_next_obj_by_type(topo, HWLOC_OBJ_NUMANODE, obj))) {
		if (hwloc_bitmap_isset(cpu->nodeset, obj->os_index)) {
			break;
		}
	}
	return obj;
}

static int string_to_glibc_sched_affinity(char *_cpu_list, cpu_set_t *schedset)
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
			pr_err("%s: error: expecting number before comma: %s\n",
				__func__, _cpu_list); /* empty token */
			ret = -EINVAL;
			goto out;
		}
		if ((minus = strchr(token, '-'))) {
			int start, end;

			if (*(minus + 1) == 0) {
				pr_err("%s: error: expecting number before minus: %s\n",
					__func__, _cpu_list); /* empty token */
				ret = -EINVAL;
				goto out;
			}
			*minus = 0;
			start = atoi(token);
			end = atoi(minus + 1);
			for (i = start; i <= end; i++) {
				//pr_debug("%s: schedset[%d]=1\n", __func__, i);
				CPU_SET(i, schedset);
			}
		} else {
			//pr_debug("%s: schedset[%d]=1\n", __func__, atoi(token));
			CPU_SET(atoi(token), schedset);
		}
		token = strsep(&cpu_list, ",");
	}

	ret = 0;
 out:
	free(cpu_list);
	return ret;
}

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
	hwloc_obj_t obj = NULL;
	size_t shm_sz;
	struct stat shm_stat;
	int retry;
	unsigned int hwloc_api_version;

	/* Discover topology */
	if ((ret = hwloc_topology_init(&topo))) {
		pr_err("%s: error: hwloc_topology_init\n",
			__func__);
		return;
	}

	if ((ret = hwloc_topology_set_io_types_filter(topo, HWLOC_TYPE_FILTER_KEEP_ALL))) {
		pr_err("%s: error: hwloc_topology_set_io_types_filter\n",
			__func__);
		return;
	}

	if ((ret = hwloc_topology_load(topo))) {
		pr_err("%s: error: hwloc_topology_load\n",
			__func__);
		return;
	}

	hwloc_api_version = hwloc_get_api_version();
	if (hwloc_api_version < 0x00020000) {
		pr_err("%s: error: hwloc_api_version (%x) is lower than expected. hwloc symbols might be resolved with unexpected object (e.g. MVAPICH).\n",
		       __func__, hwloc_api_version);
		return;
	}
	
	if ((ret = hwloc_topology_abi_check(topo))) {
		pr_err("%s: error: hwloc_topology_abi_check: hwloc symbols might be resolved with unexpected object (e.g. MVAPICH).\n",
			__func__);
		return;
	}

	//pr_debug("%s: info: ncpus=%d\n", __func__, hwloc_get_nbobjs_by_type(topo, HWLOC_OBJ_PU));

	while ((obj = hwloc_get_next_obj_by_type(topo, HWLOC_OBJ_PU, obj))) {
		if (max_cpu_os_index < (int)obj->os_index) {
			max_cpu_os_index = obj->os_index;
		}
	}

	//pr_debug("%s: info: max_cpu_os_index=%d\n", __func__, max_cpu_os_index);


	/* Establish shared nthr_per_pu array */
	shm_sz = hwloc_get_nbobjs_by_type(topo, HWLOC_OBJ_PU) * sizeof(int);

	if ((shm_fd = shm_open(shm_fn, O_EXCL | O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) != -1) { /* Leader */
		if ((ftruncate(shm_fd, shm_sz))) {
			pr_err("%s: error: ftruncate: %s\n",
			       __func__, strerror(errno));
		}
		shm_leader = 1;
	} else { /* Follower */
		if (errno != EEXIST) {
			pr_err("%s: error: shm_open: %s\n",
			       __func__, strerror(errno));
			return;
		}

		if ((shm_fd = shm_open(shm_fn, O_RDWR, S_IRUSR | S_IWUSR)) == -1) {
			pr_err("%s: error: shm_open: %s\n",
			       __func__, strerror(errno));
			return;
		}
		
		/* Wait until the file becomes ready */
		retry = 0;
		while ((ret = fstat(shm_fd, &shm_stat)) != -1 &&
		       shm_stat.st_size != shm_sz) {
			retry++;
			if (retry > N_SHM_RETRY) {
				shm_unlink(shm_fn);
				pr_err("%s: error: wait on /dev/shm%s timed out\n",
				       shm_fn, __func__);
				return;
			}
			usleep(1000);
		}
		
		if (ret == -1) {
			pr_err("%s: error: fstat: %s\n",
			       __func__, strerror(errno));
			return;
		}
	}

	if ((nthr_per_pu = mmap(0, shm_sz, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)) == MAP_FAILED) {
		pr_err("%s: error: mapping nthr_per_pu: %s\n",
		       __func__, strerror(errno));
	}
}

__attribute__((destructor)) void uti_finalize()
{
	hwloc_topology_destroy(topo);

	if (shm_leader && shm_unlink(shm_fn)) {
		pr_err("%s: error: shm_unlink: %s\n",
		       __func__, strerror(errno));
	}
}

static int uti_rr_allocate_cpu(hwloc_cpuset_t cpuset)
{
	int i;
	int mincpu;
	unsigned int minrr;
	int newval;
	int oldval;

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
	int caller_cpu_os_index;
	hwloc_obj_t caller_cpu;
	hwloc_cpuset_t cpuset = NULL, tmpset = NULL, env_cpuset = NULL;
	char *env_schedset_str;
	cap_t cap = NULL;

	/* Find caller cpu for later resolution of subgroups */
	caller_cpu_os_index = sched_getcpu();
	caller_cpu = hwloc_get_pu_obj_by_os_index(topo, caller_cpu_os_index);
	if (caller_cpu) {
		pr_debug("caller_cpu os_index=%d,logical_index=%d\n",
				 caller_cpu->os_index, caller_cpu->logical_index);
	}

	if (!caller_cpu) {
                pr_err("%s: error: cpu with os_index of %d not found\n",
			__func__, caller_cpu_os_index);
		ret = -ENOENT;
                goto out;
	}

	/* Initial cpuset */
	cpuset = hwloc_bitmap_dup(hwloc_topology_get_allowed_cpuset(topo));

	if (!(tmpset = hwloc_bitmap_alloc())) {
                pr_err("%s: error: hwloc_bitmap_alloc\n",
			__func__);
		ret = -ENOMEM;
		goto out;
	}

	/* NUMA_SET */
	if (uti_attr->flags & UTI_FLAG_NUMA_SET) {
		hwloc_bitmap_zero(tmpset);
		for_each_node(i, uti_attr->numa_set) {
			hwloc_obj_t numanode = hwloc_get_numanode_obj_by_os_index(topo, i);
			pr_debug("%s: info: numa os_index: %d, logical_index: %d\n",
					 __func__, i, numanode ? numanode->logical_index: -1);
			if (numanode) {
				pr_cpuset("numanode->cpuset", numanode->cpuset);
				hwloc_bitmap_or(tmpset, tmpset, numanode->cpuset);
			}
		}
		hwloc_bitmap_and(cpuset, cpuset, tmpset);
	}

	
	/* {SAME,DIFFERENT}_NUMA_DOMAIN */
	if (uti_attr->flags &
	    (UTI_FLAG_SAME_NUMA_DOMAIN | UTI_FLAG_DIFFERENT_NUMA_DOMAIN)) {
		hwloc_obj_t caller_numanode =
			get_numanode_obj_by_pu(caller_cpu);
		pr_debug("%s: info: caller_numanode->os_index=%d\n", __func__, caller_numanode ? caller_numanode->os_index : -1);
		
		if (caller_numanode) {
			if (uti_attr->flags & UTI_FLAG_SAME_NUMA_DOMAIN) {
				hwloc_bitmap_and(cpuset, cpuset, caller_numanode->cpuset);
			}
			
			if (uti_attr->flags & UTI_FLAG_DIFFERENT_NUMA_DOMAIN) {
				hwloc_bitmap_andnot(cpuset, cpuset, caller_numanode->cpuset);
			}
		}
	}

	/* {SAME,DIFFERENT}_L1 */
	if (uti_attr->flags &
	    (UTI_FLAG_SAME_L1 | UTI_FLAG_DIFFERENT_L1)) {
		hwloc_obj_t l1 =
			get_cache_obj_by_pu(HWLOC_OBJ_L1CACHE, caller_cpu);
		
		if (l1) {
			pr_cpuset("l1->cpuset", l1->cpuset);

			if (uti_attr->flags & UTI_FLAG_SAME_L1) {
				hwloc_bitmap_and(cpuset, cpuset, l1->cpuset);
			}
			
			if (uti_attr->flags & UTI_FLAG_DIFFERENT_L1) {
				hwloc_bitmap_andnot(cpuset, cpuset, l1->cpuset);
			}
		} else {
			pr_warn("%s: Caller L1 not found\n", __func__);
		}
	}

	/* {SAME,DIFFERENT}_L2 */
	if (uti_attr->flags &
	    (UTI_FLAG_SAME_L2 | UTI_FLAG_DIFFERENT_L2)) {
		hwloc_obj_t l2 =
			get_cache_obj_by_pu(HWLOC_OBJ_L2CACHE, caller_cpu);
		
		if (l2) {
			pr_cpuset("l2->cpuset", l2->cpuset);

			if ((uti_attr->flags & UTI_FLAG_SAME_L2) && l2) {
				hwloc_bitmap_and(cpuset, cpuset, l2->cpuset);
			}
			
			if ((uti_attr->flags & UTI_FLAG_DIFFERENT_L2) && l2) {
				hwloc_bitmap_andnot(cpuset, cpuset, l2->cpuset);
			}
		} else {
			pr_warn("%s: Caller L2 not found\n", __func__);
		}
	}

	/* {SAME,DIFFERENT}_L3 */
	if (uti_attr->flags &
	    (UTI_FLAG_SAME_L3 | UTI_FLAG_DIFFERENT_L3)) {
		hwloc_obj_t l3 =
			get_cache_obj_by_pu(HWLOC_OBJ_L3CACHE, caller_cpu);
		
		if (l3) {
			pr_cpuset("l3->cpuset", l3->cpuset);

			if ((uti_attr->flags & UTI_FLAG_SAME_L3) && l3) {
				hwloc_bitmap_and(cpuset, cpuset, l3->cpuset);
			}
			
			if ((uti_attr->flags & UTI_FLAG_DIFFERENT_L3) && l3) {
				hwloc_bitmap_andnot(cpuset, cpuset, l3->cpuset);
			}
		} else {
			pr_warn("%s: Caller L3 not found\n", __func__);
		}
	}

	/* UTI_CPU_SET, PREFER_FWK, PREFER_LWK */
	if (!(env_cpuset = hwloc_bitmap_alloc())) {
                pr_err("%s: error: allocating env_cpuset\n",
			__func__);
		ret = -ENOMEM;
		goto out;
	}

	env_schedset_str = getenv("UTI_CPU_SET");
	if (env_schedset_str) {
		cpu_set_t env_schedset;

		CPU_ZERO(&env_schedset);
		if (!(ret = string_to_glibc_sched_affinity(env_schedset_str, &env_schedset))) {
			hwloc_cpuset_from_glibc_sched_affinity(topo, env_cpuset, &env_schedset, sizeof(cpu_set_t));

		} else {
			pr_warn("%s: warning: string_to_glibc_sched_affinity returned %d\n",
				__func__, ret);
		}
	}
	
	if (!hwloc_bitmap_iszero(env_cpuset)) {
		pr_cpuset("cpuset", cpuset);
		pr_cpuset("env_cpuset", env_cpuset);
		
		if ((uti_attr->flags & UTI_FLAG_PREFER_LWK)) {
			hwloc_bitmap_andnot(cpuset, cpuset, env_cpuset);
		} else { /* Including PREFER_FWK and !PREFER_FWK */
			hwloc_bitmap_and(cpuset, cpuset, env_cpuset);
		}
	}

	/* FABRIC_INTR_AFFINITY */
	if (uti_attr->flags & UTI_FLAG_FABRIC_INTR_AFFINITY) {
		struct uti_fabric *uti_fabric;

		for (uti_fabric = uti_fabrics; uti_fabric->name; uti_fabric++) { 
			hwloc_obj_t fabric, package;

			pr_debug("uti_fabric->name: %s\n", uti_fabric->name);
			fabric = hwloc_ibv_get_device_osdev_by_name(topo, uti_fabric->name);

			/* Find non-I/O ancestor which have cpuset */
			if (fabric) {
				package = hwloc_get_non_io_ancestor_obj(topo, fabric);
				if (package) {
					pr_cpuset("package", package->cpuset);
					hwloc_bitmap_and(cpuset, cpuset, package->cpuset);
				}
			}
		}
	}

	/* EXCLUSIVE_CPU, CPU_INTENSIVE */
	if (uti_attr->flags &
	    (UTI_FLAG_EXCLUSIVE_CPU | UTI_FLAG_CPU_INTENSIVE)) {
		pr_cpuset("rr cpuset", cpuset);
		hwloc_bitmap_only(cpuset, uti_rr_allocate_cpu(cpuset));
	}

	/* Write cpuset to pthread_attr_t */
	ret = hwloc_bitmap_weight(cpuset);
	if (ret > 0) {
		cpu_set_t schedset;

		hwloc_cpuset_to_glibc_sched_affinity(topo, cpuset, &schedset, sizeof(schedset));
		pr_schedset("final schedset", &schedset);

		if ((ret = pthread_attr_setaffinity_np(pthread_attr, sizeof(cpu_set_t), &schedset))) {
			pr_err("%s: error: pthread_setaffinity_np\n",
				__func__);
			goto out;
		}
	} else {
		pr_warn("%s: warning: cpuset is empty\n", __func__);
	}

	/* Assign real-time scheduler */
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

		/* Process needs CAP_SYS_NICE or positive RLIMIT_RTPRIO to use it */
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
	if (cpuset)
		hwloc_bitmap_free(cpuset);
	if (tmpset)
		hwloc_bitmap_free(tmpset);
	if (env_cpuset)
		hwloc_bitmap_free(env_cpuset);
	return ret;
}

int uti_pthread_create(pthread_t *thread, pthread_attr_t *_pthread_attr,
                       void *(*start_routine) (void *), void *arg,
		       uti_attr_t *uti_attr)
{
	int ret;
	char *disable_uti_str;
	int disable_uti;
	pthread_attr_t *pthread_attr;
	pthread_attr_t *tmp_attr = NULL;
	
	disable_uti_str = getenv("DISABLE_UTI");
	disable_uti = disable_uti_str ? atoi(disable_uti_str) : 0;

	if (disable_uti) {
		pr_warn("%s: warning: uti is disabled\n",
			 __func__);
	}

	if (!uti_attr) {
		pr_debug("%s: info: uti_attr is NULL\n",
			 __func__);
	}

	if (disable_uti) {
		pthread_attr = _pthread_attr;
	} else if (uti_attr) {
		if (!_pthread_attr) {
			if (!(tmp_attr = malloc(sizeof(pthread_attr_t)))) {
				pr_err("%s: error: allocating tmp_attr\n",
				       __func__);
				ret = ENOMEM;
				goto out;
			}
			pthread_attr_init(tmp_attr);
			pthread_attr = tmp_attr;
		} else {
			pthread_attr = _pthread_attr;
		}

		if ((ret = uti_set_pthread_attr(pthread_attr, uti_attr))) {
			pr_err("%s: error: uti_set_pthread_attr\n",
			       __func__);
			goto out;
		}
	}

	if ((ret = pthread_create(thread, pthread_attr, start_routine, arg))) {
		pr_err("%s: error: pthread_create: %s\n",
		       __func__, strerror(ret));
		if (!(ret = syscall(731, 1, uti_attr))) {
			pr_warn("%s: warning: libuti.so for Linux is used\n",
				__func__);
		}
		goto out;
	}

	ret = 0;
out:
	if (tmp_attr) {
		pthread_attr_destroy(tmp_attr);
		free(tmp_attr);
	}

	return ret;
}
