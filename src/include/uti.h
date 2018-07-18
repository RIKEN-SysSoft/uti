/**
 * \file uti.h.in
 *  License details are found in the file LICENSE.
 * \brief
 *  UTI API
 * \author Masamichi Takagi <masamichi.takagi@riken.jp> \par
 *      Copyright (C) 2016-2017 RIKEN AICS
 */

#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#define UTI_FLAG_NUMA_SET (1ULL << 1) /* Indicates NUMA_SET is specified */

#define UTI_FLAG_SAME_NUMA_DOMAIN (1ULL << 2)
#define UTI_FLAG_DIFFERENT_NUMA_DOMAIN (1ULL << 3)

#define UTI_FLAG_SAME_L1 (1ULL << 4)
#define UTI_FLAG_SAME_L2 (1ULL << 5)
#define UTI_FLAG_SAME_L3 (1ULL << 6)

#define UTI_FLAG_DIFFERENT_L1 (1ULL << 7)
#define UTI_FLAG_DIFFERENT_L2 (1ULL << 8)
#define UTI_FLAG_DIFFERENT_L3 (1ULL << 9)

#define UTI_FLAG_EXCLUSIVE_CPU (1ULL << 10)
#define UTI_FLAG_CPU_INTENSIVE (1ULL << 11)
#define UTI_FLAG_HIGH_PRIORITY (1ULL << 12)
#define UTI_FLAG_NON_COOPERATIVE (1ULL << 13)

#define UTI_FLAG_PREFER_LWK (1ULL << 14)
#define UTI_FLAG_PREFER_FWK (1ULL << 15)
#define UTI_FLAG_FABRIC_INTR_AFFINITY (1ULL << 16)

/* Linux default value is used */
#define UTI_MAX_NUMA_DOMAINS (1024)

typedef struct uti_attr {
    /* UTI_CPU_SET environmental variable is used to denote the preferred location of utility thread */
    uint64_t numa_set[(UTI_MAX_NUMA_DOMAINS + sizeof(uint64_t) * 8 - 1) / (sizeof(uint64_t) * 8)];
    uint64_t flags; /* Representing location and behavior hints by bitmap */
} uti_attr_t;

enum UTI_LOGLEVEL {
	UTI_LOGLEVEL_ERR = 0,
	UTI_LOGLEVEL_WARN,
	UTI_LOGLEVEL_DEBUG
};

extern int loglevel;

int uti_attr_init(uti_attr_t *attr);
int uti_attr_destroy(uti_attr_t *attr);

static inline int UTI_ATTR_NUMA_SET(uti_attr_t *uti_attr, uint64_t *nodemask, size_t maxnode) {
	if (!uti_attr) {
		return EINVAL;
	} else {
		memcpy(uti_attr->numa_set, nodemask, (UTI_MAX_NUMA_DOMAINS + 7) / 8);
		uti_attr->flags |= UTI_FLAG_NUMA_SET;
	}
	return 0;
}

static inline int UTI_ATTR_SAME_NUMA_DOMAIN(uti_attr_t *uti_attr)
{
	if (!uti_attr) {
		return EINVAL;
	} else {
		uti_attr->flags |= UTI_FLAG_SAME_NUMA_DOMAIN;
	}
	return 0;
}

static inline int UTI_ATTR_DIFFERENT_NUMA_DOMAIN(uti_attr_t *uti_attr)
{
	if (!uti_attr) {
		return EINVAL;
	} else {
		uti_attr->flags |= UTI_FLAG_DIFFERENT_NUMA_DOMAIN;
	}
	return 0;
}

static inline int UTI_ATTR_SAME_L1(uti_attr_t *uti_attr)
{
	if (!uti_attr) {
		return EINVAL;
	} else {
		uti_attr->flags |= UTI_FLAG_SAME_L1;
	}
	return 0;
}

static inline int UTI_ATTR_SAME_L2(uti_attr_t *uti_attr)
{
	if (!uti_attr) {
		return EINVAL;
	} else {
		uti_attr->flags |= UTI_FLAG_SAME_L2;
	}
	return 0;
}


static inline int UTI_ATTR_SAME_L3(uti_attr_t *uti_attr)
{
	if (!uti_attr) {
		return EINVAL;
	} else {
		uti_attr->flags |= UTI_FLAG_SAME_L3;
	}
	return 0;
}


static inline int UTI_ATTR_DIFFERENT_L1(uti_attr_t *uti_attr)
{
	if (!uti_attr) {
		return EINVAL;
	} else {
		uti_attr->flags |= UTI_FLAG_DIFFERENT_L1;
        }
	return 0;
}

static inline int UTI_ATTR_DIFFERENT_L2(uti_attr_t *uti_attr)
{
	if (!uti_attr) {
		return EINVAL;
	} else {
		uti_attr->flags |= UTI_FLAG_DIFFERENT_L2;
	}
	return 0;
}

static inline int UTI_ATTR_DIFFERENT_L3(uti_attr_t *uti_attr)
{
	if (!uti_attr) {
		return EINVAL;
	} else {
		uti_attr->flags |= UTI_FLAG_DIFFERENT_L3;
	}
	return 0;
}

static inline int UTI_ATTR_EXCLUSIVE_CPU(uti_attr_t *uti_attr)
{
	if (!uti_attr) {
		return EINVAL;
	} else {
		uti_attr->flags |= UTI_FLAG_EXCLUSIVE_CPU;
	}
	return 0;
}

static inline int UTI_ATTR_CPU_INTENSIVE(uti_attr_t *uti_attr)
{
	if (!uti_attr) {
		return EINVAL;
	} else {
		uti_attr->flags |= UTI_FLAG_CPU_INTENSIVE;
        }
	return 0;
}

static inline int UTI_ATTR_HIGH_PRIORITY(uti_attr_t *uti_attr)
{
	if (!uti_attr) {
		return EINVAL;
	} else {
		uti_attr->flags |= UTI_FLAG_HIGH_PRIORITY;
        }
	return 0;
}

static inline int UTI_ATTR_NON_COOPERATIVE(uti_attr_t *uti_attr)
{
	if (!uti_attr) {
		return EINVAL;
	} else {
		uti_attr->flags |= UTI_FLAG_NON_COOPERATIVE;
	}
	return 0;
}

static inline int UTI_ATTR_PREFER_LWK(uti_attr_t *uti_attr)
{
	if (!uti_attr) {
		return EINVAL;
	} else {
		uti_attr->flags |= UTI_FLAG_PREFER_LWK;
	}
	return 0;
}

static inline int UTI_ATTR_PREFER_FWK(uti_attr_t *uti_attr)
{
	if (!uti_attr) {
		return EINVAL;
	} else {
		uti_attr->flags |= UTI_FLAG_PREFER_FWK;
	}
	return 0;
}

static inline int UTI_ATTR_FABRIC_INTR_AFFINITY(uti_attr_t *uti_attr)
{
	if (!uti_attr) {
		return EINVAL;
	} else {
		uti_attr->flags |= UTI_FLAG_FABRIC_INTR_AFFINITY;
	}
	return 0;
}

int uti_pthread_create(pthread_t *thread, pthread_attr_t * attr,
                       void *(*start_routine) (void *), void * arg,
                       uti_attr_t *uti_attr);

int uti_set_loglevel(enum UTI_LOGLEVEL level);
