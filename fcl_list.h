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

   Example:
   #include <stdlib.h>
   #include "fcl_list.h"
   struct my_node {
     struct fcl_list_links links;
     int id;
     int priority;
   };
   // generate the functions for my_node
   FCL_LIST_DL_DEFINE(node, struct my_node, links)
   void main() {
     // allocate and initialize the list head
     struct fcl_list_links *head = malloc(sizeof(*head));
     fcl_list_dl_init(head);
     // allocate and add 10 nodes to the list
     struct my_node *nodes = malloc(sizeof(*nodes) * 10);
     int i;
     for (i=0; i < 10; i++)
       node_list_insert_tail(head, &nodes[i]);
     // iterate over the list, printing and then removing the nodes
     struct fcl_list_links *iter, *tmp;
     struct my_node *entry;
     FCL_LIST_DL_EACH(head, iter, tmp) {
       entry = node_list_get_entry(iter);
       print_node(entry);
       node_list_remove(entry);
     }
     free(head);
     free(nodes);
     return 0;
   }
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

// NOTE: it is safe to remove elements while iterating with this macro
// l = ptr to initialized fcl_list_links struct
// i, tmp = fcl_list_links ptrs (may be NULL)
#define FCL_LIST_DL_EACH(l, i, tmp)                              \
  for (i = (l)->next; (i != (l)) && (tmp = i->next); i = (tmp)) 


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
