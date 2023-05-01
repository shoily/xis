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
#define KPAGE_FLAGS_FREE_BUDDY 256
#define KPAGE_FLAGS_FREE_SUB 512

#define ERR_NOMEM -1
#define ERR_INVALID -2
#define ERR_NOT_SUPPORTED -3
#define ERR_SUCCESS 0
#define ERR_RETRY 1

#define KPAGE_TO_PHYS_ADDR(page) (((page) - kpages) << PAGE_SHIFT)

struct kpage {
    // CONVERT_64
    u8 order;
    u16 flags;
    u32 refcount;
    void *virt_addr;
    spinlock lock;
    union {
        struct cache_desc *desc; // for actual cache page
        u32 free_slot_count; // for cache metadata
        struct list adjacent_pages;
        struct list free_page;
    };
};

extern struct kpage *kpages;

// 4K, 8K, 16K, 32K, 64K, 128K, 256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M, 64M, 128M, 512M, 1G
enum page_alloc_order {
                       ALLOC_ORDER_4K,
                       ALLOC_ORDER_8K,
                       ALLOC_ORDER_16K,
                       ALLOC_ORDER_32K,
                       ALLOC_ORDER_64K,
                       ALLOC_ORDER_128K,
                       ALLOC_ORDER_256K,
                       ALLOC_ORDER_512K,
                       ALLOC_ORDER_1MB,
                       ALLOC_ORDER_2MB,
                       ALLOC_ORDER_4MB,
                       ALLOC_ORDER_8MB,
                       ALLOC_ORDER_16MB,
                       ALLOC_ORDER_32MB,
                       ALLOC_ORDER_64MB,
                       ALLOC_ORDER_128MB,
                       ALLOC_ORDER_256MB,
                       ALLOC_ORDER_512MB,
                       ALLOC_ORDER_1GB,
                       MAX_ALLOC_ORDER = ALLOC_ORDER_1GB
};

#define ORDER_TO_PAGE_SIZE(order) (PAGE_SIZE << (order))
#define NPAGES_TO_ORDER(npages) ({s32 __clz_order__ = 31-__builtin_clz((npages)); \
            (__clz_order__ != __builtin_ctz((npages))) ? __clz_order__+1 :  __clz_order__;})

s32 mem_init();
struct kpage *page_alloc(u32 order);
void *page_alloc_kmap_linear(u32 order);
void page_free_linear(void *addr);
void page_free_user(void *addr);

#endif
