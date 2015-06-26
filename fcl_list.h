/*!
  \file
  \copyright Copyright (c) 2015, Richard Fujiyama
  Licensed under the terms of the New BSD license.
*/

/* A simple, header-only linked list library.
   Typesafety is provided by generating type-specific functions via a macro.
   Allocation and deallocation are not managed by this library and are the
   responsibility of the caller.
   This library is NOT thread safe.

   The fcl_list_link struct, together with FCL_LIST_FIFO_XXX macros implement
   a singly-linked list where items are inserted at the tail and removed from
   the head.  Insert (push), remove (pop), and get (peek) operations are O(1).

   The fcl_list_links struct, together with FCL_LIST_DL_XXX macros implement
   a doubly-linked list with a tail pointer in the head for O(1) tail access.

   Advanced usage:
   Object reuse:
   A struct with embedded fcl_list_links may have both
   FCL_LIST_DL_XXX and FCL_LIST_FIFO_XXX functions generated such that objects
   in use are doubly linked, while unused objects are placed on a singly linked
   free list.
   Multiple links:
   The FCL_LIST_XXX_DEFINE macros may be used multiple times for the same
   struct as long as the name is unique.  A struct with multiple embedded link
   structs can thus be on multiple lists at the same time.
*/

#ifndef _FCL_LIST_H_
#define _FCL_LIST_H_

#include <assert.h>   // assert
#include <stddef.h>   // offsetof

#ifndef CONTAINER_OF
#define CONTAINER_OF(ptr, type, field) \
  ((type*)((char*)(ptr) - offsetof(type, field)))
#endif


struct fcl_list_link {
  struct fcl_list_link *next;
};

struct fcl_list_links {
  struct fcl_list_links *next;
  struct fcl_list_links *prev;
};


#define FCL_LIST_FIFO_EACH(h, i, tmp)                              \
  for (i = (h)->first; (i) && (tmp = i->next, 1); i = (tmp))

// name = list prefix, eg events
// type = container type, eg event
// field_type = the type in container containing the list link(s)
// field = name of the field_type struct in the container
#define FCL_LIST_FIFO_DEFINE(name, type, field_type, field) \
struct name##_list_head {\
  field_type *first; \
  field_type *last; \
};  \
void name##_list_head_init(struct name##_list_head *head) {\
  assert(head); \
  head->first = NULL; \
} \
type *name##_list_get_entry(field_type *e) {\
  assert(e);  \
  return CONTAINER_OF(e, type, field);  \
} \
int name##_list_is_empty(struct name##_list_head *head) {\
  assert(head); \
  return head->first ? 0 : 1; \
} \
void name##_list_insert(struct name##_list_head *head, type *e) {\
  assert(head); \
  assert(e);  \
  if (! name##_list_is_empty(head)) { \
    head->last->next = &e->field;  \
  } else {  \
    head->first = &e->field;  \
  } \
  head->last = &e->field; \
  e->field.next = NULL; \
} \
type *name##_list_get(struct name##_list_head *head) {\
  assert(head); \
  if (!name##_list_is_empty(head)) \
    return name##_list_get_entry(head->first); \
  return NULL;  \
} \
type *name##_list_remove(struct name##_list_head *head) {\
  assert(head); \
  type *tmp;  \
  if (!name##_list_is_empty(head)) {  \
    tmp = name##_list_get_entry(head->first); \
    head->first = head->first->next;  \
    return tmp; \
  } \
  return NULL;  \
}


static inline void fcl_list_dl_init(struct fcl_list_links *head) {
  assert(head);
  head->next = head;
  head->prev = head;
}

// NOTE: it is safe to remove elements while iterating with this macro
// l = ptr to initialized fcl_list_links struct
// i, tmp = fcl_list_links ptrs (may be NULL)
#define FCL_LIST_DL_EACH(l, i, tmp)                              \
  for (i = (l)->next; (i != (l)) && (tmp = i->next); i = (tmp)) 

// name = list prefix, eg events
// type = container type, eg event
// field = name of the fcl_list_links struct in the container
#define FCL_LIST_DL_DEFINE(name, type, field) \
void name##_list_insert_head(struct fcl_list_links *head, type *e) {\
  assert(head); \
  assert(e);  \
  e->field.prev = head; \
  e->field.next = head->next; \
  head->next->prev = &e->field; \
  head->next = &e->field; \
} \
void name##_list_insert_tail(struct fcl_list_links *head, type *e) {\
  assert(head); \
  assert(e);  \
  e->field.prev = head->prev;  \
  e->field.next = head;  \
  head->prev->next = &e->field; \
  head->prev = &e->field; \
} \
void name##_list_insert_after(type *current, type *e) {\
  assert(current);  \
  assert(e);  \
  e->field.prev = &current->field;  \
  e->field.next = current->field.next;  \
  current->field.next->prev = &e->field;  \
  current->field.next = &e->field;  \
} \
void name##_list_insert_before(type *current, type *e) {\
  assert(current);  \
  assert(e);  \
  e->field.prev = current->field.prev;  \
  e->field.next = &current->field;  \
  current->field.prev->next = &e->field;  \
  current->field.prev = &e->field;  \
} \
void name##_list_remove(type *e) {\
  assert(e);  \
  e->field.prev->next = e->field.next;  \
  e->field.next->prev = e->field.prev;  \
} \
type *name##_list_get_entry(struct fcl_list_links *e) {\
  assert(e);  \
  return CONTAINER_OF(e, type, field);  \
} \
type *name##_list_get_first(struct fcl_list_links *head) {\
  assert(head); \
  if (head->next == head) \
    return NULL;  \
  return name##_list_get_entry(head->next); \
} \
type *name##_list_get_last(struct fcl_list_links *head) {\
  assert(head); \
  if (head->prev == head) \
    return NULL;  \
  return name##_list_get_entry(head->prev); \
} \
int name##_list_is_empty(struct fcl_list_links *head) {\
  assert(head); \
  return head->next == head;  \
}


#endif  // _FCL_LIST_H_
