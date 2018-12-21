
#ifndef list_head_h
#define list_head_h

#include "mydef.h"

struct list_head {
    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) do { \
    (ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

static void inline __list_add(struct list_head * new1,
                       struct list_head * prev,
                       struct list_head * next)
{
    next->prev = new1;
    new1->next = next;
    new1->prev = prev;
    prev->next = new1;
}

#define list_add(new1, head) \
do { \
    __list_add(new1, head, (head)->next); \
} while (0)

#define list_add_tail(new1, head) \
do { \
    __list_add(new1, (head)->prev, head); \
} while (0)

static void inline __list_del(struct list_head * prev,
                       struct list_head * next)
{
    next->prev = prev;
    prev->next = next;
}

#define list_del(entry) \
do { \
    __list_del((entry)->prev, (entry)->next); \
} while (0)

#define list_del_init(entry) \
do { \
    __list_del((entry)->prev, (entry)->next); \
    INIT_LIST_HEAD((entry)); \
} while (0)

static int  inline list_empty(struct list_head *head)
{
    return head->next == head;
}

#define list_entry(ptr, type, member) container_of(ptr, type, member)

#endif
