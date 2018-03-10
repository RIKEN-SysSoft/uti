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
	char *disable = getenv("DISABLE_UTI");

	if (!disable) {
		rc = syscall(731, 1, uti_attr);
		if (rc != 0) {
			printf("%s: WARNING: util_indicate_clone is not available in this kernel\n");
			disable = "1";
		}
	}
	rc = pthread_create(thread, attr, start_routine, arg);
	if (rc != 0) {
		goto out;
	}

	if (!disable) {
		syscall(731, 0, NULL);
	}
 out:
	return rc;
}
