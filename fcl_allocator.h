/*!
  \file
  \copyright Copyright (c) 2015, Richard Fujiyama
  Licensed under the terms of the New BSD license.
*/

/* A simple, header-only struct allocator library.
   Typesafety is provided by generating type-specific functions via a macro.
   Allocation and deallocation are managed by this library.
   This library is NOT thread safe.
   Requires C11 support due to aligned_alloc.

   The recycle policy controls the order in which structs allocated or returned
   to the allocator are available for use.  The out of memory (oom) policy
   controls whether or not more memory is allocated when there are no structs
   available for use, and how much memory is allocated if an allocation occurs.
   Allocators may or may not support every recycle and oom policy.

   FCL_ALLOCATOR_LL implements an allocator for structs with an embedded
   fcl linked list (either singly or doubly linked).  It supports the FIFO and
   LIFO recycle policies, and the ERROR, DOUBLE, and INCREMENTAL oom policies.

   Via the optional element initialization callback, FCL_ALLOCATOR_LL maintains
   an invariant where all elements on the free list are always in the
   initialized state.
*/

#ifndef _FCL_ALLOCATOR_H_
#define _FCL_ALLOCATOR_H_

#include <stdint.h>     // uint32_t
#include <assert.h>     // assert
#include <stdlib.h>     // aligned_alloc
#include "fcl_list.h"

#ifndef LEVEL1_DCACHE_LINESIZE
/*! Used to align and place objects to avoid false sharing

  On modern Intel x86 CPUs, this is typically 64 bytes
  On Linux it can be defined while compiling with
  \verbatim -DLEVEL1_DCACHE_LINESIZE=`getconf LEVEL1_DCACHE_LINESIZE` \endverbatim
*/
#define LEVEL1_DCACHE_LINESIZE 64
#endif

#define FCL_ALLOCATOR_LL_DEFAULT_ALLOCATIONS 8

typedef enum fcl_allocator_recycle_policy {
  FCL_ALLOCATOR_RECYCLE_POLICY_NONE,
  FCL_ALLOCATOR_RECYCLE_POLICY_FIFO,
  FCL_ALLOCATOR_RECYCLE_POLICY_LIFO
} fcl_allocator_recycle_policy;

typedef enum fcl_allocator_oom_policy {
  FCL_ALLOCATOR_OOM_POLICY_NONE,
  FCL_ALLOCATOR_OOM_POLICY_ERROR,
  FCL_ALLOCATOR_OOM_POLICY_DOUBLE,
  FCL_ALLOCATOR_OOM_POLICY_INCREMENTAL
} fcl_allocator_oom_policy;


// name = allocator prefix, eg node
// type = container type, eg struct my_node
// field_type = the list link(s) type, eg struct fcl_list_links
// field = the name of the field_type struct in the container, eg links
// recycle_policy must be either FIFO or LIFO
// example usage:
// FCL_ALLOCATOR_LL_DEFINE(node, struct my_node, struct fcl_list_links, links,
//                         FIFO)
#define FCL_ALLOCATOR_LL_DECLARE(name, type, field_type, field, recycle_policy) \
FCL_LIST_##recycle_policy##_DECLARE(name##_free, type, field_type, field) \
typedef void (*fcl_allocator_elem_init_fn)(type *);  \
struct name##_allocator { \
  struct name##_free_list_head free_list; \
  size_t free_count; \
  size_t total_count; \
  size_t increment; \
  type** allocations; \
  fcl_allocator_elem_init_fn elem_init; \
  uint32_t num_allocations; \
  fcl_allocator_oom_policy oom_policy;  \
};  \
int name##_allocator_init(struct name##_allocator *a, size_t initial_size, \
                          fcl_allocator_oom_policy oom_policy, size_t inc, \
                          fcl_allocator_elem_init_fn elem_init); \
void name##_allocator_freeall(struct name##_allocator *a);  \
int _##name##_allocator_allocate(struct name##_allocator *a); \
type *name##_allocator_borrow(struct name##_allocator *a);  \
void name##_allocator_return(struct name##_allocator *a, type *e);

#define FCL_ALLOCATOR_LL_DEFINE(name, type, field_type, field, recycle_policy) \
FCL_LIST_##recycle_policy##_DEFINE(name##_free, type, field_type, field) \
int name##_allocator_init(struct name##_allocator *a, size_t initial_size, \
                          fcl_allocator_oom_policy oom_policy, size_t inc, \
                          fcl_allocator_elem_init_fn elem_init) {  \
  assert(a);  \
  type *new_structs;  \
  size_t i; \
  a->allocations = calloc(FCL_ALLOCATOR_LL_DEFAULT_ALLOCATIONS, sizeof(type*));\
  if (!a->allocations)  \
    return -1;  \
  new_structs = aligned_alloc(LEVEL1_DCACHE_LINESIZE, \
                              sizeof(*new_structs) * initial_size); \
  if (!new_structs) \
    return -1;  \
  a->allocations[0] = new_structs;  \
  name##_free_list_head_init(&a->free_list); \
  a->free_count = initial_size;  \
  a->total_count = initial_size; \
  a->elem_init = elem_init;  \
  a->oom_policy = oom_policy; \
  a->num_allocations = FCL_ALLOCATOR_LL_DEFAULT_ALLOCATIONS;  \
  switch(a->oom_policy) { \
    case FCL_ALLOCATOR_OOM_POLICY_DOUBLE:  \
      a->increment = initial_size; \
      break;  \
    case FCL_ALLOCATOR_OOM_POLICY_INCREMENTAL: \
      a->increment = inc; \
      break;  \
    default:  \
      a->increment = 0; \
  } \
  for (i=0; i < initial_size; i++) {\
    if (a->elem_init) \
      a->elem_init(&new_structs[i]);  \
    name##_free_list_insert(&a->free_list, &new_structs[i]); \
  } \
  return 1; \
} \
void name##_allocator_freeall(struct name##_allocator *a) { \
  assert(a);  \
  assert(a->allocations); \
  size_t i; \
  for (i=0; a->allocations[i]; i++) \
    free(a->allocations[i]);  \
  free(a->allocations); \
} \
int _##name##_allocator_allocate(struct name##_allocator *a) { \
  assert(a);  \
  type *new_structs;  \
  size_t i; \
  new_structs = aligned_alloc(LEVEL1_DCACHE_LINESIZE, \
                              sizeof(*new_structs) * a->increment); \
  if (!new_structs) \
    return -1;  \
  for (i=0; i < a->increment; i++) \
    name##_free_list_insert(&a->free_list, &new_structs[i]); \
  for (i=0; i < a->num_allocations; i++) {  \
    if (a->allocations[i])  \
      continue; \
    a->allocations[i] = new_structs;  \
    a->total_count += a->increment; \
    a->free_count += a->increment;  \
    return 1; \
  } \
  void *new_allocations = realloc(a->allocations, sizeof(type*) * \
                                  a->num_allocations * 2);  \
  if (new_allocations) {  \
    a->num_allocations *= 2;  \
    a->allocations = new_allocations; \
    a->total_count += a->increment; \
    a->free_count += a->increment;  \
    a->allocations[i+1] = new_structs;  \
    for (i = i+2; i < a->num_allocations; i++) \
      a->allocations[i] = NULL; \
    return 1; \
  } else {  \
    free(new_structs);  \
    return -1; \
  } \
} \
type *name##_allocator_borrow(struct name##_allocator *a) {  \
  assert(a);  \
  type *new_struct; \
  if (a->free_count == 0) { \
    switch(a->oom_policy) { \
      case FCL_ALLOCATOR_OOM_POLICY_DOUBLE: \
        if (_##name##_allocator_allocate(a) == 1) \
          a->increment *= 2;  \
        break;  \
      case FCL_ALLOCATOR_OOM_POLICY_INCREMENTAL: \
        _##name##_allocator_allocate(a);  \
        break;  \
      default:  \
        return NULL;  \
    } \
  } \
  new_struct = name##_free_list_remove(&a->free_list);  \
  if (new_struct) \
    a->free_count--;  \
  return new_struct;  \
} \
void name##_allocator_return(struct name##_allocator *a, type *e) {  \
  assert(a);  \
  assert(e);  \
  if (a->elem_init) \
    a->elem_init(e);  \
  name##_free_list_insert(&a->free_list, e);  \
  a->free_count++;  \
}



#endif  // _FCL_ALLOCATOR_H_
