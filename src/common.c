#include "uti.h"
#include "uti_impl.h"

int loglevel = UTI_LOGLEVEL_DEBUG;

/*
 * Messages with level below or equal to loglevel
 * are printed out
 */
int uti_set_loglevel(enum UTI_LOGLEVEL level)
{
	loglevel = level;
	return 0;
}
