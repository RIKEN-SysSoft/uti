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
#include "uti.h"

int uti_attr_init(uti_attr_t *attr) {
	bzero(attr, sizeof(uti_attr_t));
    return 0;
}

int uti_attr_destroy(uti_attr_t *attr) {
	/* Do nothing */
    return 0;
}

int uti_pthread_create(pthread_t *thread, const pthread_attr_t * attr,
                       void *(*start_routine) (void *), void * arg, uti_attr_t *uti_attr) {
	int rc;
	char *disable_uti, *uti_os_kind;

	disable_uti = getenv("DISABLE_UTI");
	//printf("%s: enter,disable=%s\n", __FUNCTION__, disable_uti ? disable_uti : "NULL");

	if (!disable_uti) {
		rc = syscall(731, 1, uti_attr);
		if (rc != 0) {
			printf("%s: WARNING: util_indicate_clone is not available in this kernel\n", __FUNCTION__);
			disable_uti = "1";
		}
	}

#if 0
	/* Allocation and scheduling using UTI_CPU_SET */

	uti_os_kind = getenv("UTI_OS_KIND");
	if (!uti_os_kind) {
		uti_os_kind = "Linux";
	}

	if (!strcmp(uti_os_kind, "Linux")) {
		char *uti_cpu_set_str;
		uti_cpu_set_str = getenv("MY_ASYNC_PROGRESS_PIN");
		if (!async_progress_pin_str) {
			printf("%s: ERROR: MY_ASYNC_PROGRESS_PIN not found\n", __FUNCTION__);
			goto sub_out;
		}
		
		list = async_progress_pin_str;
		while (1) {
			token = strsep(&list, ",");
			if (!token) {
				break;
			}
			progress_cpus[n_progress_cpus++] = atoi(token);
		}
		
		rank_str = getenv("PMI_RANK");
		if (!rank_str) {
			printf("%s: ERROR: PMI_RANK not found\n", __FUNCTION__);
			goto sub_out;
		}
		rank = atoi(rank_str);

		CPU_ZERO(&cpuset);
		CPU_SET(progress_cpus[rank % n_progress_cpus], &cpuset);
	}
#endif

	rc = pthread_create(thread, attr, start_routine, arg);
	if (rc != 0) {
		goto out;
	}

	if (!disable_uti) {
		syscall(731, 0, NULL);
	}
 out:
	return rc;
}
