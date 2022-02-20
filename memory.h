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
#include "lock.h"

#define KPAGE_FLAGS_FREE 1
#define KPAGE_FLAGS_ALLOCATED 2
#define KPAGE_FLAGS_RESERVED 4
#define KPAGE_FLAGS_KERNEL 8
#define KPAGE_FLAGS_FREEABLE 16
#define KPAGE_FLAGS_NOT_FREEABLE 32
#define KPAGE_FLAGS_CACHE_METADATA 64
#define KPAGE_FLAGS_CACHE 128

#define KPAGE_FLAGS_MASK 0xffffffff

#define ERR_NOMEM -1
#define ERR_INVALID -2
#define ERR_NOT_SUPPORTED -3
#define ERR_SUCCESS 0
#define ERR_RETRY 1

#define KPAGE_TO_PHYS_ADDR(page) (((page) - kpages) << PAGE_SHIFT)

struct kpage {
    // CONVERT_64
    u8 npages;
    u8 flags;
    u32 refcount;
    void *virt_addr;
    spinlock lock;
    union {
        struct cache_desc *desc; // for actual cache page
        u32 free_slot_count; // for cache metadata
        struct list adjacent_pages;
    };
};

extern struct kpage *kpages;

s32 mem_init();
struct kpage *page_alloc(u32 npages);
void *page_alloc_kmap_linear(u32 npages);
void page_free(void *addr);

#endif
