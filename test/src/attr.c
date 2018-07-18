#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sched.h>
#include <uti.h>
#include <util.h>

void print_sched(char *name)
{
	int sched;

	sched = sched_getscheduler(0);
	printf("[INFO] %s is running on cpu %d with sched %d (SCHED_FIFO: %d)\n",
	       name, sched_getcpu(), sched, SCHED_FIFO);
}

void *util_thread(void *arg)
{
	print_sched("Child");
	return NULL;
}

void thread_test(uti_attr_t *uti_attr, char *msg)
{
	pthread_t thr;
	int ret;

	print_sched("Parent");

	ret = uti_pthread_create(&thr, NULL, util_thread, NULL, uti_attr);
	NG(ret == 0, "pthread_create\n");

	pthread_join(thr, NULL);

	printf("[ DO ] %s\n", msg);
 out:;
}

int
main(int argc, char **argv)
{
	int ret;
	uti_attr_t uti_attr;
	uint64_t *numa_set;

	numa_set = calloc(UTI_MAX_NUMA_DOMAINS, sizeof(uint64_t));
	NG(numa_set, "allocating numa_set");

	/* NUMA_SET */
	uti_attr_init(&uti_attr);

	memset(numa_set, 0, UTI_MAX_NUMA_DOMAINS * sizeof(uint64_t));
	numa_set[0] = 2; // NUMA node #1
	ret = UTI_ATTR_NUMA_SET(&uti_attr, numa_set, UTI_MAX_NUMA_DOMAINS * sizeof(uint64_t));
	OKNG(ret == 0, "UTI_ATTR_NUMA_SET\n");

	thread_test(&uti_attr, "Check if child is using NUMA#1 CPU");

	/* NUMA_SET | EXCLUSIVE */
	uti_attr_init(&uti_attr);

	memset(numa_set, 0, UTI_MAX_NUMA_DOMAINS * sizeof(uint64_t));
	numa_set[0] = 2; // NUMA node #1

	ret = UTI_ATTR_NUMA_SET(&uti_attr, numa_set, UTI_MAX_NUMA_DOMAINS * sizeof(uint64_t));
	OKNG(ret == 0, "UTI_ATTR_NUMA_SET\n");

	ret = UTI_ATTR_EXCLUSIVE_CPU(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_EXCLUSIVE_CPU\n");

	thread_test(&uti_attr, "Check if child is given least loaded CPU of NUMA#1");

	/* NUMA_SET | EXCLUSIVE */
	uti_attr_init(&uti_attr);

	memset(numa_set, 0, UTI_MAX_NUMA_DOMAINS * sizeof(uint64_t));
	numa_set[0] = 2; // NUMA node #1

	ret = UTI_ATTR_NUMA_SET(&uti_attr, numa_set, UTI_MAX_NUMA_DOMAINS * sizeof(uint64_t));
	OKNG(ret == 0, "UTI_ATTR_NUMA_SET\n");

	ret = UTI_ATTR_EXCLUSIVE_CPU(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_EXCLUSIVE_CPU\n");

	thread_test(&uti_attr, "Check if child is given least loaded CPU of NUMA#1");

	/* SAME_NUMA_DOMAIN */
	uti_attr_init(&uti_attr);

	ret = UTI_ATTR_SAME_NUMA_DOMAIN(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_SAME_NUMA_DOMAIN\n");

	thread_test(&uti_attr, "Check if child is sharing NUMA domain with parent");


	/* SAME_NUMA_DOMAIN | CPU_INTENSIVE */
	uti_attr_init(&uti_attr);

	ret = UTI_ATTR_SAME_NUMA_DOMAIN(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_SAME_NUMA_DOMAIN\n");

	ret = UTI_ATTR_CPU_INTENSIVE(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_CPU_INTENSIVE\n");

	thread_test(&uti_attr, "Check if RR pointer of parent NUMA domain proceeded");

	/* SAME_NUMA_DOMAIN | CPU_INTENSIVE */
	uti_attr_init(&uti_attr);

	ret = UTI_ATTR_SAME_NUMA_DOMAIN(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_SAME_NUMA_DOMAIN\n");

	ret = UTI_ATTR_CPU_INTENSIVE(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_CPU_INTENSIVE\n");

	thread_test(&uti_attr, "Check if RR pointer of parent NUMA domain proceeded");

	/* DIFFERENT_NUMA_DOMAIN */
	uti_attr_init(&uti_attr);

	ret = UTI_ATTR_DIFFERENT_NUMA_DOMAIN(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_DIFFERENT_NUMA_DOMAIN\n");
	thread_test(&uti_attr, "Check if child isn't sharing NUMA domain with parent");

	/* DIFFERENT_NUMA_DOMAIN | HIGH_PRIORITY */
	uti_attr_init(&uti_attr);

	ret = UTI_ATTR_DIFFERENT_NUMA_DOMAIN(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_DIFFERENT_NUMA_DOMAIN\n");

	ret = UTI_ATTR_HIGH_PRIORITY(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_HIGH_PRIORITY\n");

	thread_test(&uti_attr, "Check if child is running on different NUMA domain and using SCHED_FIFO");

	/* SAME_L1 */
	uti_attr_init(&uti_attr);

	ret = UTI_ATTR_SAME_L1(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_SAME_L1\n");

	thread_test(&uti_attr, "Check if child is sharing L1 with parent");

	/* SAME_L1 | NON_COOPERATIVE */
	uti_attr_init(&uti_attr);

	ret = UTI_ATTR_SAME_L1(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_SAME_L1\n");

	ret = UTI_ATTR_NON_COOPERATIVE(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_NON_COOPERATIVE\n");

	thread_test(&uti_attr, "Check if child is sharing L1 with parent (NON_COOPERATIVE has no effect)");

	/* SAME_L2 */
	uti_attr_init(&uti_attr);

	ret = UTI_ATTR_SAME_L2(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_SAME_L2\n");

	thread_test(&uti_attr, "Check if child is sharing L2 with parent");

	/* SAME_L2 | CPU_INTENSIVE */
	uti_attr_init(&uti_attr);

	ret = UTI_ATTR_SAME_L2(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_SAME_L2\n");

	ret = UTI_ATTR_CPU_INTENSIVE(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_CPU_INTENSIVE\n");

	thread_test(&uti_attr, "Check if child is given least loaded CPU of parent L2 group");

	/* SAME_L3 */
	uti_attr_init(&uti_attr);

	ret = UTI_ATTR_SAME_L3(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_SAME_L3\n");

	thread_test(&uti_attr, "Check if child is sharing L3 with parent");

	
	/* SAME_L3 | CPU_INTENSIVE */
	uti_attr_init(&uti_attr);

	ret = UTI_ATTR_SAME_L3(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_SAME_L3\n");

	ret = UTI_ATTR_CPU_INTENSIVE(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_CPU_INTENSIVE\n");

	thread_test(&uti_attr, "Check if child is given least loaded CPU of parent L3 group");


	/* DIFFERENT_L1 */
	uti_attr_init(&uti_attr);

	ret = UTI_ATTR_DIFFERENT_L1(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_DIFFERENT_L1\n");

	thread_test(&uti_attr, "Check if child isn't sharing L1 with parent");

	/* DIFFERENT_L1 | CPU_INTENSIVE */
	uti_attr_init(&uti_attr);

	ret = UTI_ATTR_DIFFERENT_L1(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_DIFFERENT_L1\n");

	ret = UTI_ATTR_CPU_INTENSIVE(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_CPU_INTENSIVE\n");

	thread_test(&uti_attr, "Check if child is given least loaded CPU of complement of parent L1 group");

	/* DIFFERENT_L2 */
	uti_attr_init(&uti_attr);

	ret = UTI_ATTR_DIFFERENT_L2(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_DIFFERENT_L2\n");

	thread_test(&uti_attr, "Check if child isn't sharing parent L2 group");

	/* DIFFERENT_L2 | CPU_INTENSIVE */
	uti_attr_init(&uti_attr);

	ret = UTI_ATTR_DIFFERENT_L2(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_DIFFERENT_L2\n");

	ret = UTI_ATTR_CPU_INTENSIVE(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_CPU_INTENSIVE\n");

	thread_test(&uti_attr, "Check if child is given least loaded CPU of complement of parent L2 group");

	/* DIFFERENT_L3 */
	uti_attr_init(&uti_attr);

	ret = UTI_ATTR_DIFFERENT_L3(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_DIFFERENT_L3\n");

	thread_test(&uti_attr, "Check if child isn't sharing parent L3 group");

	/* DIFFERENT_L3 | CPU_INTENSIVE */
	uti_attr_init(&uti_attr);

	ret = UTI_ATTR_DIFFERENT_L3(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_DIFFERENT_L3\n");

	ret = UTI_ATTR_CPU_INTENSIVE(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_CPU_INTENSIVE\n");

	thread_test(&uti_attr, "Check if child is given least loaded CPU of complement of parent L3 group");

	/* UTI_CPU_SET */
	setenv("UTI_CPU_SET", "0-2,4", 1);
	uti_attr_init(&uti_attr);
	
	printf("[INFO] UTI_CPU_SET\n");

	thread_test(&uti_attr, "Check if child is running on CPU in {0-2,4}");
	unsetenv("UTI_CPU_SET");

	/* PREFER_FWK */
	setenv("UTI_CPU_SET", "0-2,4", 1);
	uti_attr_init(&uti_attr);

	ret = UTI_ATTR_PREFER_FWK(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_PREFER_FWK\n");

	thread_test(&uti_attr, "Check if child is running on CPU in {0-2,4}");
	unsetenv("UTI_CPU_SET");

	/* PREFER_LWK */
	setenv("UTI_CPU_SET", "0,3-4", 1);
	uti_attr_init(&uti_attr);

	ret = UTI_ATTR_PREFER_LWK(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_PREFER_LWK\n");

	thread_test(&uti_attr, "Check if child is running on CPU not in {0,3-4}");
	unsetenv("UTI_CPU_SET");

	/* FABRIC_INTR_AFFINITY */
	uti_attr_init(&uti_attr);

	ret = UTI_ATTR_FABRIC_INTR_AFFINITY(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_FABRIC_INTR_AFFINITY\n");

	thread_test(&uti_attr, "Check if child is running on CPU of fabric group");

	ret = 0;
out:

	uti_attr_destroy(&uti_attr);
	return ret;
}
