/*!
  \file
  \copyright Copyright (c) 2015, Richard Fujiyama
  Licensed under the terms of the New BSD license.
*/

/* Simple, stand alone pointer manipulation macros */
#ifndef _FCL_MACRO_H_
#define _FCL_MACRO_H_

// returns a pointer to a container of type @type given the pointer @ptr which
// is the field @field in the container
// ex: ptr = &container.field, and this returns &container
#ifndef FCL_CONTAINER_OF
#define FCL_CONTAINER_OF(ptr, type, field) \
  ((type*)((char*)(ptr) - offsetof(type, field)))
#endif

// returns a void pointer @offset bytes past @ptr
// ex: ptr = &container_struct, offset = sizeof(member_struct), and this
// returns a void pointer inside the container_struct
#ifndef FCL_PTR_PAST
#define FCL_PTR_PAST(ptr, offset)  \
  ((void*)((char*)(ptr) + (offset)))
#endif

#endif  // _FCL_MACRO_H_

