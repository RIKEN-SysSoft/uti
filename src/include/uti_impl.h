#ifndef _UTI_IMPL_H_INCLUDED_
#define _UTI_IMPL_H_INCLUDED_

#include <hwloc.h>
#include <uti.h>

/* Log */

#define pr_level(level, fmt, args...) do {	\
	if (uti_loglevel >= level) {	\
		fprintf(stdout, fmt, ##args);	\
	}					\
} while (0)

#define pr_err(fmt, args...) pr_level(UTI_LOGLEVEL_ERR, fmt, ##args)
#define pr_warn(fmt, args...) pr_level(UTI_LOGLEVEL_WARN, fmt, ##args)
#define pr_debug(fmt, args...) pr_level(UTI_LOGLEVEL_DEBUG, fmt, ##args)


/* Misc */

#define UTI_NWORDS_NUMA_SET ((UTI_MAX_NUMA_DOMAINS + sizeof(uint64_t) * 8 - 1) / (sizeof(uint64_t) * 8))

struct uti_fabric {
	char *name;
	hwloc_obj_t obj;
};

extern struct uti_fabric uti_fabrics[];

extern cpu_set_t cpu_active_mask;

#endif /* _UTI_IMPL_H_INCLUDED_ */
