
#ifndef mydef_h
#define mydef_h

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "my_errno.h"
#include "VEPlayerLogs.h"

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <winsock2.h>
#include <mswsock.h>
#include <mmsystem.h>

#pragma warning(disable: 4217)
#pragma warning(disable: 4996)
#pragma warning(disable: 4273)
#pragma warning(disable: 4312)
#pragma comment(lib, "mswsock.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "pthreadVSE2.lib")
#pragma comment(lib, "winmm.lib")

#ifdef dll_import
#define visible __declspec(dllimport)
#else
#define visible __declspec(dllexport)
#endif

#define inline __inline
#define __inline__ __inline
#define __attribute__(x)
#define __builtin_expect(x, y) (x)

static inline int posix_memalign(void **memptr, size_t alignment, size_t size)
{
    *memptr = _aligned_malloc(size, alignment);
    if (*memptr == NULL) {
        return -1;
    }
    return 0;
}
#define posix_free _aligned_free

#define usleep(x) Sleep((x) / 1000)

static int __sync_bool_compare_and_swap(void** lkp, void* cmp_value, void* new_value)
{
    void* n = InterlockedCompareExchangePointer((volatile PVOID*) lkp, new_value, cmp_value);
    return (n == cmp_value);
}

#define __sync_val_compare_and_swap(lkp, cmp_value, new_value)  \
    InterlockedCompareExchange ((LONG *) (lkp), (LONG) (new_value), (LONG) (cmp_value))

#define __sync_add_and_fetch(lkp, val) \
    (val + ((int) InterlockedExchangeAdd((LONG *) (lkp), (LONG) val)))

#define __sync_sub_and_fetch(lkp, val) __sync_add_and_fetch((lkp), -(val))
#define __sync_lock_test_and_set(lkp, val) (void *) InterlockedExchangePointer((void **) (lkp), (void *) (val))

#else
#define visible
#include "unistd.h"

#ifdef __ANDROID__
#include <android/log.h>
#include <malloc.h>

static inline int posix_memalign(void** pp, size_t align, size_t size)
{
    *pp = memalign(align, size);
    if (*pp == NULL) {
        return -1;
    }
    return 0;
}
#endif
#define posix_free free
#endif

#ifdef __cplusplus
#define capi extern "C" visible
#define capiw extern "C" visible __attribute__((weak))
#else
#define capi extern visible
#define capiw extern visible __attribute__((weak))
#endif

#define my_min(x, y) ((x) < (y) ? (x) : (y))
#define my_max(x, y) ((x) > (y) ? (x) : (y))
#define event_number(x) (0 == (x & (x - 1)))

#define elements(x) (sizeof(x) / sizeof(x[0]))
#define container_of(ptr, type, member) ((type *) ((char *) ptr - offsetof(type, member)))
typedef void (*free_t) (void*);
typedef int (*printf_t) (const char* fmt, ...);
#define roundup(bytes, align) (((bytes) + (align) - 1) & (~((align) - 1)))

#ifndef _MSC_VER
    capi void logbuffer(const char * __restrict, ...);
    #define logmsg(fmt, args...) \
    do { \
        logbuffer(fmt, ##args); \
    } while (0)
#else
    #define logmsg printf
#endif

#if 0
    #define mark(...) (void) 0
#else
    #define mark(fmt, args...) JPLAYER_LOG_TRACE("%d@%s " fmt "\n", __LINE__, __FUNCTION__, ##args)
#endif

#if !defined(NDEBUG)
#if 0
    #define ctrace(x) do {mark(#x " begin"); x; mark(#x " end");} while (0)
#else
    #define ctrace(x) do {x;} while (0)
#endif
    #define logmsg_d(...) JPLAYER_LOG_TRACE(__VA_ARGS__)
#else
    #define ctrace(x) do {x;} while (0)
    #define logmsg_d(...) (void) 0
#endif

#if defined(__arm__)
#define my_abort() __asm("bkpt 0")
#elif defined(__arm64__)
#define my_abort() asm("brk 0")
#elif defined(_MSC_VER)
#define my_abort() asm{"int 3"}
#else
#define my_abort() asm("int $3")
#endif

#if defined(NDEBUG)
#define my_assert(x) ((void) 0)
#else
#define my_assert(x) \
do { \
    if (!(x)) { \
        my_abort(); \
    } \
} while (0)
#endif

#define trace_interval(fmt, args...) \
do { \
    static uint32_t x = 0; \
    int cur = now(); \
    if (x == 0) { \
        x = cur; \
    } \
    JPLAYER_LOG_TRACE("%d@%s: %d " fmt "\n", __LINE__, __FUNCTION__, cur - x, ##args); \
    x = cur; \
} while (0)

#define trace_change(x, msg) \
do { \
    static int64_t y = 0; \
    const char* message = msg; \
    if (message == NULL) { \
        message = #x; \
    } \
    if (y != x) { \
        JPLAYER_LOG_TRACE("%s: %s changed from %lld(%llx) to %lld(%llx)\n", \
               __FUNCTION__, message, y, y, (int64_t) x, (int64_t) x); \
        y = (int64_t) x; \
    } \
} while (0)

#define logpcm_enable 0
#define logpcm(x, buf, len) \
do { \
    extern const char* pcm_path;\
    static FILE* file = NULL; \
    if (pcm_path != NULL) { \
        if (file == NULL) { \
            char buf[1204] = {0}; \
            sprintf(buf, "%s%s", pcm_path, x); \
            file = fopen(buf, "wb"); \
            if (file == NULL) { \
                JPLAYER_LOG_TRACE("failed to open %s, errno: %d\n", buf, errno); \
            } else { \
                JPLAYER_LOG_TRACE("opened %s %p\n", buf, file); \
            } \
        } \
        \
        if (file != NULL) { \
            fwrite(buf, 1, len, file); \
        } \
    } else if (file != NULL) { \
        JPLAYER_LOG_TRACE("close %p\n", file); \
        fclose(file); \
        file = NULL; \
    } \
} while (0)

#endif
