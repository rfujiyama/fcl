/*!
  \file
  \copyright Copyright (c) 2015, Richard Fujiyama
  Licensed under the terms of the New BSD license.
*/

/* A simple, header-only linked list library.
   Typesafety is provided by generating type-specific functions via a macro.

   The fcl_list_links struct, together with FCL_LIST_DL_XXX macros implement
   a doubly-linked list with a tail pointer in the head for O(1) tail access.

   WARNING: this library is NOT thread safe
*/

#ifndef _FCL_LIST_H_
#define _FCL_LIST_H_

#include <assert.h>   // assert
#include <stddef.h>   // offsetof

#ifndef CONTAINER_OF
#define CONTAINER_OF(ptr, type, field) \
  ((type*)((char*)(ptr) - offsetof(type, field)))
#endif

struct fcl_list_links {
  struct fcl_list_links *next;
  struct fcl_list_links *prev;
};


static inline void fcl_list_dl_init(struct fcl_list_links *head) {
  assert(head);
  head->next = head;
  head->prev = head;
}


// TODO: check insert and removal of a single element
// TODO: single element next field initialized
// name = list prefix, eg events
// type = container type, eg event
// field_type = the type in container containing the list link(s)
// field = name of the field_type struct in the container
#define FCL_LIST_FIFO_DEFINE(name, type, field_type, field) \
struct name##_list_head {\
  field_type *first; \
  field_type *last; \
};  \
inline void name##_list_head_init(struct name##_list_head *head) {\
  assert(head); \
  head->first = NULL; \
} \
type *name##_list_get_entry(field_type *e) {\
  assert(e);  \
  return CONTAINER_OF(e, type, field);  \
} \
inline int name##_list_is_empty(struct name##_list_head *head) {\
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
  if (name##_list_is_empty(head)) \
    return NULL;  \
  return name##_list_get_entry(head->first); \
} \
type *name##_list_remove(struct name##_list_head *head) {\
  assert(head); \
  type *tmp = name##_list_get(head);  \
  if (tmp)  \
    head->first = head->first->next; \
  return tmp; \
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
