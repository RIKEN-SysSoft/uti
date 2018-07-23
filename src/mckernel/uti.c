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
#include "uti_impl.h"

int uti_attr_init(uti_attr_t *attr) {
	memset(attr, 0, sizeof(uti_attr_t));
	return 0;
}

int uti_attr_destroy(uti_attr_t *attr) {
	return 0;
}

int uti_pthread_create(pthread_t *thread, pthread_attr_t *attr,
                       void *(*start_routine) (void *), void * arg,
		       uti_attr_t *uti_attr) {
	int ret;
	char *disable_uti_str;
	int disable_uti;

	disable_uti_str = getenv("DISABLE_UTI");
	disable_uti = disable_uti_str ? atoi(disable_uti_str) : 0;

	if (disable_uti) {
		pr_warn("%s: warning: uti is disabled\n",
			 __func__);
	}

	if (!disable_uti) {
		if ((ret = syscall(731, 1, uti_attr))) {
			pr_warn("%s: warning: util_indicate_clone is not available in this kernel\n",
				__func__);
			disable_uti = 1;
		}
	}

	if ((ret = pthread_create(thread, attr, start_routine, arg))) {
		goto out;
	}

 out:
	if (!disable_uti) {
		syscall(731, 0, NULL);
	}

	return ret;
}
