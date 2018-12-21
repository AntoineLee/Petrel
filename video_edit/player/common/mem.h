
#ifndef mem_h
#define mem_h

#include <stdlib.h>
#include "mydef.h"

#define debug_malloc(x, y, z) malloc(x)
#define my_malloc malloc
#define my_free free
#define mem_bytes() 0
#define mem_stats(x) ((void) (0))
#define my_tell(x) ((void) (0))
#define check_memory(x) ((void) (0))


#define heap_alloc(x, nb) (typeof(*x)*) my_malloc(sizeof(*x) + nb)
#define heap_free my_free

#endif
