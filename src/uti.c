#include <stdbool.h>
#include "uti.h"

int uti_attr_init(uti_attr_t *attr) {
    return 0;
}

int uti_attr_destroy(uti_attr_t *attr) {
    return 0;
}

int uti_pthread_create(pthread_t *thread, const pthread_attr_t * attr,
                       void *(*start_routine) (void *), void * arg, uti_attr_t *uti_attr) {
    return pthread_create(thread, attr, start_routine, arg);
}
