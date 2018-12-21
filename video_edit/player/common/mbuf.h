
#ifndef mbuf_h
#define mbuf_h

#include "mydef.h"
#include "my_buffer.h"

capi struct my_buffer* do_mbuf_alloc_2(uintptr_t bytes, const char* func, int line);

#define mbuf_alloc_2(bytes) do_mbuf_alloc_2(bytes, __FUNCTION__, __LINE__)

#endif
