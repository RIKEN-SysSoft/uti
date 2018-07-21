#ifndef _UTI_IMPL_H_INCLUDED_
#define _UTI_IMPL_H_INCLUDED_

#include <hwloc.h>

#define pr_err(fmt, args...) do {		\
	if (loglevel >= UTI_LOGLEVEL_ERR) {	\
		fprintf(stdout, fmt, ##args);	\
	}					\
} while (0)

#define pr_warn(fmt, args...) do {		\
	if (loglevel >= UTI_LOGLEVEL_WARN) {	\
		fprintf(stdout, fmt, ##args);	\
	}					\
} while (0)

#define pr_debug(fmt, args...) do {		\
	if (loglevel >= UTI_LOGLEVEL_DEBUG) {	\
		fprintf(stdout, fmt, ##args);	\
	}					\
} while (0)

#define UTI_NWORDS_NUMA_SET ((UTI_MAX_NUMA_DOMAINS + sizeof(uint64_t) * 8 - 1) / (sizeof(uint64_t) * 8))

struct uti_fabric {
	char *name;
	hwloc_obj_t obj;
};

extern struct uti_fabric uti_fabrics[];

extern cpu_set_t cpu_active_mask;
extern int loglevel;

#endif /* _UTI_IMPL_H_INCLUDED_ */
