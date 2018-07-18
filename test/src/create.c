#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sched.h>
#include <uti.h>
#include <util.h>

pthread_barrier_t bar;
pthread_t thr;

void *util_thread(void *arg) {
	int ret;

	ret = syscall(732);
	OKNGNOJUMP(ret == -1, "Child is running on Linux\n");

	printf("[INFO] child cpu: %d\n", sched_getcpu());

	pthread_barrier_wait(&bar);

out:
	return NULL;
}

int main(int argc, char **argv) {
	int ret;
	uti_attr_t uti_attr;

	pthread_barrier_init(&bar, NULL, 2);

	ret = syscall(732);
	if (ret == -1) {
		printf("[INFO] Parent is running on Linux\n");
	} else {
		printf("[INFO] Parent is running on McKernel\n");
	}

	printf("[INFO] parent cpu: %d\n", sched_getcpu());

	ret = uti_attr_init(&uti_attr);
	OKNG(ret == 0, "uti_attr_init returned %d\n", ret);

	ret = UTI_ATTR_SAME_NUMA_DOMAIN(&uti_attr);
	OKNG(ret == 0, "UTI_ATTR_SAME_NUMA_DOMAIN\n");
	
	ret = uti_pthread_create(&thr, NULL, util_thread, NULL, &uti_attr);
	OKNG(ret == 0, "uti_pthread_create\n");

	ret = uti_attr_destroy(&uti_attr);
	OKNG(ret == 0, "uti_attr_destroy\n");
	
	pthread_barrier_wait(&bar);
	
	ret = pthread_join(thr, NULL);
	OKNG(ret == 0, "pthread_join\n");

	ret = 0;
 out:
	return ret;
}
