#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <signal.h>
#include "uti.h"

pthread_mutex_t mutex;
pthread_cond_t cond;
int passed, flag;
pthread_t thr;

void *util_thread(void *arg) {
	int rc;
	unsigned long mem;
	int tid;
	int lineno = 101;

	rc = syscall(732);
	if (rc == -1)
		fprintf(stderr, "[OK] uti-01%03d: running on Linux \n", lineno++);
	else {
		fprintf(stderr, "[NG] uti-01%03d: running on Linux (%d)\n", lineno++, rc);
	}

	tid = syscall(SYS_gettid);
	fprintf(stderr, "[INFO] uti-01%03d: tid=%d\n", lineno++, tid);

	passed = 1;
	pthread_mutex_lock(&mutex);
	while(!flag) {
		pthread_cond_wait(&cond, &mutex);
	}
	flag = 0;
	pthread_mutex_unlock(&mutex);
	fprintf(stderr, "[OK] uti-01%03d: returned from pthread_cond_wait()\n", lineno++);

	return NULL;
}

int main(int argc, char **argv) {
	int rc;
	unsigned long mem;
	pthread_attr_t attr;
	uti_attr_t uti_attr;
	int tid;
	int lineno = 1;

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	fprintf(stderr, "[INFO] uti-01%03d: start\n", lineno++);

	tid = syscall(SYS_gettid);
	fprintf(stderr, "[INFO] uti-01%03d: tid=%d\n", lineno++, tid);
	
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	
	rc = uti_attr_init(&uti_attr);
	if (rc) {
		goto uti_exit;
	}
#if 0	
	/* Give a hint that it's beneficial to put the thread
	 * on the same NUMA-node as the creator */
	rc = UTI_ATTR_SAME_NUMA_DOMAIN(&uti_attr);
	if (rc) {
		goto uti_destroy_and_exit;
	}
	
	/* Give a hint that it's beneficial to put it on a CPU with lightest load. */
	rc = UTI_ATTR_CPU_INTENSIVE(&uti_attr);
	if (rc) {
		goto uti_destroy_and_exit;
	}
	
	/* Give a hint that it's beneficial to prioritize it in scheduling. */
	rc = UTI_ATTR_HIGH_PRIORITY(&uti_attr);
	if (rc) {
		goto uti_destroy_and_exit;
	}
#endif	
#if 1
	rc = uti_pthread_create(&thr, &attr, util_thread, NULL, /*&uti_attr*/NULL);
#else
	rc = pthread_create(&thr, &attr, util_thread, NULL);
#endif
	if (rc) {
		goto uti_destroy_and_exit;
	}

	fprintf(stderr, "[OK] uti-01%03d: returned from uti_pthread_create\n", lineno++);
	
 uti_destroy_and_exit:
	rc = uti_attr_destroy(&uti_attr);
	if (rc) {
		goto uti_exit;
	}
	
	while (!passed) {
		asm volatile("pause" ::: "memory"); 
	}
	fprintf(stderr, "[OK] uti-01%03d: non-zero value of passed detected\n", lineno++);
	usleep(100 * 1000UL); /* Send debug message through serial takes 0.05 sec */

	pthread_mutex_lock(&mutex);
	flag = 1;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
	fprintf(stderr, "[OK] uti-01%03d: returned from pthread_mutex_unlock()\n", lineno++);

	fprintf(stderr, "[INFO] uti-01%03d: end\n", lineno++);
 uti_exit:
	exit(0);
}
