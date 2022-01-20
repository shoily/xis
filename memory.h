/*****************************************************************************/
/*  File: memory.h                                                           */
/*                                                                           */
/*  Description: Header file for memory allocation code.                     */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: July 9, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#ifndef MEMORY_H
#define MEMORY_H

#include "type.h"

#define KPAGE_FLAGS_FREE 1
#define KPAGE_FLAGS_ALLOCATED 2
#define KPAGE_FLAGS_RESERVED 4
#define KPAGE_FLAGS_KERNEL 8
#define KPAGE_FLAGS_FREEABLE 16
#define KPAGE_FLAGS_NOT_FREEABLE 32
#define KPAGE_FLAGS_CACHE_METADATA 64
#define KPAGE_FLAGS_CACHE 128

#define KPAGE_FLAGS_MASK 0xffffffff

s32 mem_init();
struct kpage *page_alloc(u32 npages);
void *page_alloc_kmap_linear(u32 npages);

#endif
