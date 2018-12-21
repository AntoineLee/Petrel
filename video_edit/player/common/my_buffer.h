
#ifndef my_buffer_h
#define my_buffer_h

#include "list_head.h"
#include "lock.h"
#include <stdint.h>

struct mem_head;
typedef intptr_t (*addref_t)(struct mem_head *, intptr_t);

struct mem_head {
    union {
        struct list_head head;
        struct {
            intptr_t ref;
            addref_t addref;
        } mem;
    } u;
};

struct mbuf_operations {
    struct my_buffer* (*clone) (struct my_buffer*);
    void (*free) (struct my_buffer*);
};

struct my_buffer {
    struct list_head head;
    struct mem_head* mem_head;
    struct mbuf_operations* mop;

    char* ptr[2];
    int64_t timestamp;
    uintptr_t length;
    int pos;
};

static inline uint32_t free_buffer(struct list_head* head)
{
    uint32_t n = 0;
    while (!list_empty(head)) {
        struct list_head* ent = head->next;
        list_del(ent);

        struct my_buffer* buf = list_entry(ent, struct my_buffer, head);
        buf->mop->free(buf);
        ++n;
    }
    return n;
}

//uint32_t inline calc_buffer_length(struct list_head* head)
//{
//    uint32_t n = 0;
//    for (struct list_head* ent = head->next; ent != head; ent = ent->next) {
//        struct my_buffer* buf = list_entry(ent, struct my_buffer, head);
//        n += buf->length;
//    }
//    return n;
//}

#endif
