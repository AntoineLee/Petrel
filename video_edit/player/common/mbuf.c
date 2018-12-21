
#include "mbuf.h"
#include "mem.h"
#include "lock.h"

struct mbuf_list {
    struct list_head head;
    lock_t llk;
    uintptr_t ref;
    uintptr_t nr, capacity;
    uintptr_t bytes;
    intptr_t clone;
};

#define prefix_length sizeof(intptr_t)
#define pool_size 64
static struct mbuf_list* buf_pool[pool_size] = {0};

#if (defined(__GNUC__) || defined(__clang__))
__attribute__((constructor))
#endif
static void mbuf_init()
{
    static struct mbuf_list obj = {
        LIST_HEAD_INIT(obj.head),
        lock_initial,
        1,
        0,
        32,
        sizeof(struct my_buffer),
        0
    };

    my_assert(offsetof(struct mem_head, u.head) == 0);
    my_assert(offsetof(struct my_buffer, head) == 0);
    buf_pool[0] = &obj;
}

static inline void* addr_base(struct list_head* headp)
{
    char* base = (char *) headp - prefix_length;
    check_memory(base);
    return base;
}


static char* alloc_from_handle(uintptr_t handle, uintptr_t bytes, const char* func, int line)
{
    struct mbuf_list* list = buf_pool[handle];
    char* pch = NULL;
    if (list->nr == 0) {
LABEL:
        pch = (char *) debug_malloc(bytes + prefix_length, func, line);
        if (pch != NULL) {
            *((intptr_t *) pch) = handle;
            pch += prefix_length;
            INIT_LIST_HEAD((struct list_head *) pch);
        }
    } else {
        struct list_head* head = &list->head;
        if (0 == try_lock_my(&list->llk)) {
            if (list->nr == 0) {
                unlock_my(&list->llk);
                goto LABEL;
            }

            struct list_head* next = head->next;
            list_del_init(next);
            --list->nr;
            unlock_my(&list->llk);

            pch = (char *) next;
        } else {
            goto LABEL;
        }
    }
    return pch;
}

static struct my_buffer* mbuf_alloc_head(const char* func, int line)
{
    struct mbuf_list* list = buf_pool[0];
    return (struct my_buffer *) alloc_from_handle(0, list->bytes, func, line);
}

static void do_free(struct list_head* addr)
{
    char* base = addr_base(addr);
    intptr_t handle = *((intptr_t *) base);
    if (handle < 0) {
        my_free(base);
        return;
    }

    struct mbuf_list* list = buf_pool[handle];
    my_assert(list != NULL);

    lock_my(&list->llk);
    if (list->nr < list->capacity && list->ref > 0) {
        list_add(addr, &list->head);
        ++list->nr;
    } else {
        my_free(base);
    }
    unlock_my(&list->llk);
}

static void mbuf_free(struct my_buffer* mbuf)
{
    if (mbuf->mem_head != NULL) {
        mbuf->mem_head->u.mem.addref(mbuf->mem_head, -1);
    }
    do_free(&mbuf->head);
}

static inline intptr_t __mem_head_addref(struct mem_head* head, intptr_t ref)
{
    my_assert(((void *) &head->u.mem) == ((void *) head));
    my_assert(((void *) &head->u.head) == ((void *) head));

    if (ref > 0) {
        ref = __sync_add_and_fetch(&head->u.mem.ref, ref);
    } else if (ref < 0) {
        ref = -ref;
        ref = __sync_sub_and_fetch(&head->u.mem.ref, ref);
    }

    my_assert(ref >= 0);
    return ref;
}

static intptr_t mem_head_addref_1(struct mem_head* head, intptr_t ref)
{
    ref = __mem_head_addref(head, ref);
    if (ref == 0) {
        do_free(&head->u.head);
    }
    return ref;
}

//static struct my_buffer* mbuf_clone(struct my_buffer* buf)
//{
//    struct mem_head* p = buf->mem_head;
//    if (p == NULL || p->u.mem.addref == NULL) {
//        return NULL;
//    }
//
//    struct my_buffer* mbuf = mbuf_alloc_head(__FUNCTION__, __LINE__);
//    if (mbuf == NULL) {
//        return NULL;
//    }
//
//    p->u.mem.addref(p, 1);
//    mbuf->mem_head = p;
//
//    mbuf->length = buf->length;
//    mbuf->ptr[0] = buf->ptr[0];
//    mbuf->ptr[1] = buf->ptr[1];
//    mbuf->mop = buf->mop;
//    return mbuf;
//}


static struct mbuf_operations mop_clone = {
    /*mbuf_clone*/NULL, mbuf_free
};


struct my_buffer* do_mbuf_alloc_2(uintptr_t bytes, const char* func, int line)
{
    intptr_t len = bytes;
    my_assert(len > 0);

    struct mem_head* p = NULL;
    struct my_buffer* mbuf = mbuf_alloc_head(func, line);
    if (mbuf == NULL) {
        return NULL;
    }

    size_t total = bytes + sizeof(struct mem_head) + prefix_length;
    char* pch = (char *) debug_malloc(total, func, line);
    if (pch != NULL) {
        *((intptr_t *) pch) = -len;

        p = (struct mem_head *) (pch + prefix_length);
        p->u.mem.ref = 1;
        p->u.mem.addref = mem_head_addref_1;

        mbuf->mem_head = p;
        mbuf->length = bytes;
        mbuf->ptr[0] = mbuf->ptr[1] = (char *) (p + 1);
        mbuf->mop = &mop_clone;
    } else {
        mbuf->mem_head = NULL;
        mbuf_free(mbuf);
        mbuf = NULL;
    }
    return mbuf;
}

