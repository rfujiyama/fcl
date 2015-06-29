/*!
  \file
  \copyright Copyright (c) 2015, Richard Fujiyama
  Licensed under the terms of the New BSD license.
*/

#ifndef _FCL_ALLOCATOR_H_
#define _FCL_ALLOCATOR_H_

#include <assert.h>     // assert
#include <stdlib.h>     // malloc
#include "fcl_list.h"

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


#define FCL_ALLOCATOR_LL_DEFINE(name, type, field_type, field, recycle_policy, oom_policy) \
FCL_LIST_##recycle_policy##_DEFINE(name##_free, type, field_type, field) \
struct name##_allocator { \
  struct name##_free_list_head free_list; \
  size_t free_count; \
  size_t total_count; \
  fcl_allocator_oom_policy oom_policy;  \
  size_t increment; \
  type** allocations; \
  size_t num_allocations; \
};  \
int name##_allocator_init(struct name##_allocator *a, size_t n, \
                          fcl_allocator_oom_policy oom_policy, size_t inc) {  \
  assert(a);  \
  type *new_structs;  \
  size_t i; \
  a->allocations = calloc(FCL_ALLOCATOR_LL_DEFAULT_ALLOCATIONS, sizeof(type*));\
  if (!a->allocations)  \
    return -1;  \
  new_structs = malloc(sizeof(*new_structs) * n); \
  if (!new_structs) \
    return -1;  \
  a->allocations[0] = new_structs;  \
  name##_free_list_head_init(&a->free_list); \
  a->free_count = n;  \
  a->total_count = n; \
  a->oom_policy = oom_policy; \
  a->num_allocations = FCL_ALLOCATOR_LL_DEFAULT_ALLOCATIONS;  \
  switch(a->oom_policy) { \
    case FCL_ALLOCATOR_OOM_POLICY_DOUBLE:  \
      a->increment = n; \
      break;  \
    case FCL_ALLOCATOR_OOM_POLICY_INCREMENTAL: \
      a->increment = inc; \
      break;  \
    default:  \
      a->increment = 0; \
  } \
  for (i=0; i < n; i++) \
    name##_free_list_insert(&a->free_list, &new_structs[i]); \
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
  new_structs = malloc(sizeof(*new_structs) * a->increment); \
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
    a->allocations[i+1] = new_structs;  \
    a->total_count += a->increment; \
    a->free_count += a->increment;  \
    return 1; \
  } else {  \
    free(new_structs);  \
    return -1; \
  } \
} \
type *name##_allocator_get(struct name##_allocator *a) {  \
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
void name##_allocator_put(struct name##_allocator *a, type *e) {  \
  assert(a);  \
  assert(e);  \
  name##_free_list_insert(&a->free_list, e);  \
  a->free_count++;  \
}



#endif  // _FCL_ALLOCATOR_H_
