/*****************************************************************************/
/*  File: memory.c                                                           */
/*                                                                           */
/*  Description: Source file for memory allocation code.                     */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: July 9, 2020                                                       */
/*        Nov 2021 - Page table initialization for the whole memory          */
/*                   (supported upto 4GB), describe each page with kpage     */
/*                   structure and keeping state (allocate/reserved/free),   */
/*                   page alloc function, memory functions are 64bit         */
/*                   compatibles, re-organize kernel memory layout in        */
/*                   kernel32.ld, adding cache support.                      */
/*                                                                           */
/*****************************************************************************/

#include "memory.h"
#include "system.h"
#include "util.h"
#include "page32.h"
#include "smp.h"

// CONVERT_64
extern addr_t _kernel_section_start;
extern addr_t _end_kernel_initial_pg_table;
extern addr_t _master_kernel_pg_dir, _kernel_pg_dir;
extern addr_t _kernel_pg_table_0;
extern addr_t init_ap_size;
extern int _end_kernel_mapped_memory;

#define MEM_CACHE_FLAG_ADDING_SLOT 1

struct mem_cache {
    u32 bitmap_size;
    u32 flags;
    u32 size;
    spinlock lock;
    struct list free_area_head;
    struct list free_slot_head;
};

struct bitmap {
    u32 size;
    char bitmap[1];
};

struct cache_desc_header{
    struct mem_cache *cache;
};

struct cache_desc {
    void *page;
    struct list free_slot_or_area;
    // bitmap
};

struct cache_fixed {
    u32 size;
    u32 chunk_size;
    struct bitmap *bitmap;
    void *addr;
};

enum KPAGE_ALLOC_MODE {
    KMAP_LINEAR_PAGE = 0,
    KMAP_NONLINEAR_PAGE,
    USER_PAGE
};

#define CACHE_MIN_CHUNK_SIZE (int)sizeof(long)
#define CACHE_MAX_NUM 10

spinlock spinlock_page_alloc;
struct mem_cache cache[CACHE_MAX_NUM] = {0};

struct kpage *kpages;
struct kpage *free_page;
struct kpage *kpage_last;
// CONVERT_64
u64 nr_kpages;
// CONVER_64
u64 total_memory;
u64 total_effective_memory;
addr_t end_kernel_data;

#define kpage_lookup(addr) (kpages+page_getpfn((addr)))

s32 mem_init() {

    u32 e820_count = *((int *) E820_MAP_VIRT_COUNT);
    struct e820_map *e820 = (struct e820_map *) E820_MAP_VIRT_ADDRESS;
    struct kpage *kpage;
    addr64_t base_aligned, addr;
    // CONVERT_64
    addr64_t kpages_memory;
    addr_t end_kernel_pgtbls;
    addr_t mp_addr;
    u64 i, j;
    u32 nr_pgtbls;
    char debug_str[80];

    INIT_SPIN_LOCK(&spinlock_page_alloc);

    // CONVERT_64
    addr = e820->base;
    // CONVERT_64
    total_effective_memory = total_memory = (e820+e820_count-1)->base + (e820+e820_count-1)->length;

    if (total_effective_memory > MAX_SUPPORTED_MEMORY)
        total_effective_memory = MAX_SUPPORTED_MEMORY;

    nr_pgtbls = (ALIGN_PGD(total_effective_memory) - _end_kernel_mapped_memory) >> PGD_SHIFT;

    nr_kpages = (total_effective_memory >> PAGE_SHIFT);
    kpages_memory = ALIGN_PAGE(nr_kpages * sizeof(struct kpage));

    _end_kernel_mapped_memory += KERNEL_VIRT_ADDR;
    end_kernel_pgtbls = ADDPTRS(_end_kernel_mapped_memory, nr_pgtbls << PAGE_SHIFT);
    kpage = kpages = (struct kpage *)end_kernel_pgtbls;

    if (ADDPTRS64(PTR64(kpages), kpages_memory) > MAX_SUPPORTED_MEMORY) {
        print_vga("Could not map page tables and kpages. Reached maximum limit for 32 bit OS. Upgrade to 64bit kernel\n");
        sprintf(debug_str, "_end_kernel_mapped_memory: %x, nr_pgtbls size: %x, kpages_memory: %x\n", PTR(_end_kernel_mapped_memory), PTR(nr_pgtbls << PAGE_SHIFT), PTR(kpages_memory));
        print_vga(debug_str);
        return -1;
    }

    if (nr_pgtbls)
        map_kernel_linear_with_pagetable(_end_kernel_mapped_memory, nr_pgtbls << PAGE_SHIFT, PTE_PRESENT | PTE_WRITE, false);

    map_kernel_linear_with_pagetable(PTR(kpages), kpages_memory, PTE_PRESENT | PTE_WRITE, false);

    end_kernel_data = ADDPTRS(kpages, kpages_memory);
    kpage_last = kpages;

    for(i=0;i<e820_count && PTR(((kpage-kpages)) < nr_kpages);i++,e820++) {
        // CONVERT_64
        base_aligned = e820->base & PAGE_MASK;
        for (j = 0; j < (((e820->length + PAGE_SIZE - 1) & PAGE_MASK) >> PAGE_SHIFT); j++) {
            if (addr == base_aligned) {
                memset(kpage, 0, sizeof(struct kpage));
                INIT_SPIN_LOCK(&kpage->lock);
                kpage->npages = 0;
                kpage->flags = (e820->type == 1) ? KPAGE_FLAGS_FREE : KPAGE_FLAGS_RESERVED;
            } else {
                addr = base_aligned;
                kpage--;
                if (e820->type != 1) {
                    kpage->flags &= ~KPAGE_FLAGS_FREE;
                    kpage->flags |= KPAGE_FLAGS_RESERVED;
                }
            }
            if (kpage->flags == KPAGE_FLAGS_FREE) {
                if (addr >= VIRT_TO_PHYS_ADDR_LINEAR(&_kernel_section_start) && addr <= VIRT_TO_PHYS_ADDR_LINEAR(end_kernel_data)) {
                    kpage->flags &= ~KPAGE_FLAGS_FREE;
                    kpage->flags |= (KPAGE_FLAGS_ALLOCATED | KPAGE_FLAGS_NOT_FREEABLE);
                }
            }
            if (free_page == NULL && kpage->flags == KPAGE_FLAGS_FREE)
                free_page = kpage;
            kpage++;

            base_aligned = addr = ADDPTRS(addr, PAGE_SIZE);
        }

        // Gap between two e820 entries. We don't know their state. Mark the pages as reserved.
        // CONVERT_64
        if ((i!=(e820_count-1)) && ((((e820+1)->base+PAGE_SIZE-1) & PAGE_MASK) != addr)) {
            // CONVERT_64
            u64 last_page_in_range = (e820+1)->base & PAGE_MASK;
            while(addr <= last_page_in_range) {
                memset(kpage, 0, sizeof(struct kpage));
                INIT_SPIN_LOCK(&kpage->lock);
                kpage->flags = KPAGE_FLAGS_RESERVED;
                kpage++;
                addr = ADDPTRS(addr, PAGE_SIZE);
            }
        }
    }

    kpages[BASE_WRITE_PAGE >> PAGE_SHIFT].flags &= ~KPAGE_FLAGS_FREE;
    kpages[BASE_WRITE_PAGE >> PAGE_SHIFT].flags |= (KPAGE_FLAGS_ALLOCATED | KPAGE_FLAGS_NOT_FREEABLE);

    map_kernel_linear_with_pagetable(ADDPTRS(KERNEL_VIRT_ADDR, BASE_WRITE_PAGE), PAGE_SIZE, PTE_PRESENT | PTE_WRITE, false);

    for(mp_addr = AP_INIT_PHYS_TEXT & PAGE_MASK; mp_addr < ALIGN_PAGE(AP_INIT_PHYS_TEXT + init_ap_size); mp_addr += PAGE_SIZE) {
        kpages[mp_addr >> PAGE_SHIFT].flags &= ~KPAGE_FLAGS_FREE;
        kpages[mp_addr >> PAGE_SHIFT].flags |= (KPAGE_FLAGS_ALLOCATED | KPAGE_FLAGS_NOT_FREEABLE);
        map_kernel_linear_with_pagetable(mp_addr, PAGE_SIZE, PTE_PRESENT | PTE_WRITE, false);
    }

    kpage_last = kpage-1;
    kpage = kpages;
    pte_t *pte;
    while (kpage != kpage_last) {
        // clear pte for the page to make it inaccessible.
        if (kpage->flags == KPAGE_FLAGS_FREE) {
            pte = ((pte_t*)&_kernel_pg_table_0) + (kpage-kpages);
            *pte = 0;
        }
        kpage++;
    }

    memset(&_kernel_pg_dir, 0, PAGE_SIZE);

    return 0;
}

struct kpage *page_alloc(u32 npages) {
    u32 i;
    u32 tmp;
    struct kpage *ptr, *head;

    if (!npages)
        return NULL;

    spinlock_lock(&spinlock_page_alloc);
    ptr = free_page;

retry:
    head = ptr;
    tmp = npages;

    while (tmp) {
        if (ptr > kpage_last) {
            ptr = NULL;
            goto out;
        }

        if (ptr->flags != KPAGE_FLAGS_FREE)
            break;

        if (free_page->flags != KPAGE_FLAGS_FREE)
            free_page = ptr;

        ptr++;
        tmp--;
    }

    if (tmp) {
        while(ptr && ptr->flags != KPAGE_FLAGS_FREE) {
            ptr++;
            if (ptr > kpage_last) {
                ptr = NULL;
                goto out;
            }
        }

        goto retry;
    }

    ptr = head;
    for (i = 0; i < npages; i++, ptr++) {
        ptr->flags &= ~KPAGE_FLAGS_FREE;
        ptr->flags |= KPAGE_FLAGS_ALLOCATED;
        ptr->refcount++;
    }
    head->npages = npages;

    if (free_page->flags != KPAGE_FLAGS_FREE) {
        free_page = ptr;
        if (free_page > kpage_last)
            free_page = kpages;
    }

out:
    spinlock_unlock(&spinlock_page_alloc);

    if (!ptr)
        return NULL;

    return head;
}

struct bitmap *cache_bitmap = NULL;
struct mem_cache *mem_cache_bitmap = NULL;

#define MEM_CACHE_ADDR_MASK (~1L)

int bitmap_firstfreebitset(u8 *bitmap, int bitmap_size) {
    int bit_position = -1;
    int num;
    int count;
    u8 oldbitmap;

    for(count=0;count<bitmap_size;count++,bitmap++) {
        oldbitmap = *bitmap;
        *bitmap |= (oldbitmap + 1);
        if (*bitmap != oldbitmap) {
            num = *bitmap - oldbitmap;
            while(num) {
                bit_position++;
                num >>= 1;
            }
            return bit_position + (count*sizeof(*bitmap));
        }
    }

    return bit_position;
}

bool bitmap_allbitsset(u8 *bitmap, int bitmap_size) {
    int count;
    for(count=0;count<bitmap_size;count++,bitmap++) {
        if (!(*bitmap & 0xff))
            return false;
    }
    return true;
}

int mem_cache_add_slot(struct mem_cache *cache) {
    struct cache_desc *desc;
    struct kpage *kpage;
    void *page;
    struct cache_desc_header *header;
    u32 slot_count = 0;

retry:
    spinlock_lock(&cache->lock);
    if (cache->flags & MEM_CACHE_FLAG_ADDING_SLOT) {
        spinlock_unlock(&cache->lock);
        goto retry;
    }

    if (!list_empty(&cache->free_slot_head)) {
        spinlock_unlock(&cache->lock);
        return 0;
    }

    cache->flags |= MEM_CACHE_FLAG_ADDING_SLOT;
    spinlock_unlock(&cache->lock);

    page = page_alloc_kmap_linear(1);
    if (!page)
        return ERR_NOMEM;

    header = (struct cache_desc_header *)page;
    header->cache = cache;

    desc = (struct cache_desc *)ADDPTRS(page, sizeof(struct cache_desc_header));
    while(ADDPTRS(desc, sizeof(struct cache_desc) + cache->bitmap_size) <= ADDPTRS(header, PAGE_SIZE)) {
        list_insert_tail(&cache->free_slot_head, &desc->free_slot_or_area);
        desc = (struct cache_desc *)ADDPTRS(desc, cache->bitmap_size);
        slot_count++;
    }

    kpage = kpage_lookup(page);
    kpage->flags |= KPAGE_FLAGS_CACHE_METADATA;
    kpage->free_slot_count = slot_count;

    spinlock_lock(&cache->lock);
    cache->flags &= ~MEM_CACHE_FLAG_ADDING_SLOT;
    spinlock_unlock(&cache->lock);

    return 0;
}

int mem_cache_init_internal(struct mem_cache *cache, u32 size) {
    INIT_SPIN_LOCK(&cache->lock);
    list_init(&cache->free_area_head);
    list_init(&cache->free_slot_head);

    cache->bitmap_size = (PAGE_SIZE / size * 8) < sizeof(long) ? sizeof(long) : (PAGE_SIZE / size * 8);

    return 0;
}

int mem_cache_init() {
    int i, j;
    for (j = 0,i = CACHE_MIN_CHUNK_SIZE; j < CACHE_MAX_NUM && i < PAGE_SIZE; j++,i<<=1) {
        cache[j].size = i;
        mem_cache_init_internal(&cache[j], i);
    }

    return 0;
}

void *mem_cache_allocate(int size) {
    struct cache_desc *desc;
    int order = 0;
    int i, err;
    int idx = 0;
    void *page;
    struct kpage *kpage;
    struct list *entry;
    u8 *bitmap;
    int first_bit;

    size = ALIGN_LONG(size);
    //desc = ADDPTRS(cache, sizeof(struct mem_cache_header));

    if (size > (PAGE_SIZE / 2)) {
        for (order = 1;
         ((size + PAGE_SIZE - 1) >> PAGE_SHIFT) > (1 << PAGE_SHIFT);
          order++);

        page = page_alloc_kmap_linear(1 << order);
        if (!page)
            return NULL;

        kpage = kpage_lookup(page);
        kpage->flags &= ~KPAGE_FLAGS_CACHE;
        kpage->desc = NULL;

        return page;
    }

    i = size;
    while(i > CACHE_MIN_CHUNK_SIZE) {
        i >>= 1;
        idx++;
    }

retry:
    spinlock_lock(&cache->lock);
    if (list_empty(&cache->free_area_head)) {
        spinlock_unlock(&cache->lock);
        err = mem_cache_add_slot(&cache[idx]);
        if (err == ERR_NOMEM) {
            idx++;
            if (cache[idx].size == 0)
                return NULL;
            goto retry;
        }
        else if (err == ERR_RETRY)
            goto retry;
        spinlock_lock(&cache->lock);
        if (list_empty(&cache->free_area_head) &&
            list_empty(&cache->free_slot_head)) {
            spinlock_unlock(&cache->lock);
            goto retry;
        }
    }

    if (!list_empty(&cache->free_area_head)) {
        entry = list_next_entry(&cache->free_area_head);
        desc = container_of(entry, struct cache_desc, free_slot_or_area);
    } else {  // !list_empty(&cache->free_slot_head)
        entry = list_next_entry(&cache->free_slot_head);
        list_remove_entry(entry);
        desc = container_of(entry, struct cache_desc, free_slot_or_area);
        spinlock_unlock(&cache->lock);
        page = page_alloc_kmap_linear(1);
        if (!page) {
            list_insert_tail(&cache->free_slot_head, entry);
            return NULL;
        }
        spinlock_lock(&cache->lock);
        memset(desc, 0, sizeof(struct cache_desc) + cache->bitmap_size);
        kpage = kpage_lookup((void*)((long)desc & PAGE_MASK));
        kpage->free_slot_count--;

        desc->page = page;
        kpage = kpage_lookup(page);
        kpage->flags |= KPAGE_FLAGS_CACHE;
        kpage->desc = desc;
        entry = &desc->free_slot_or_area;
        list_init(entry);
        list_insert_tail(&cache->free_area_head, entry);
    }
    bitmap = (u8*)ADDPTRS(desc, sizeof(struct cache_desc));

    first_bit = bitmap_firstfreebitset(bitmap, cache->bitmap_size);

    if (bitmap_allbitsset(bitmap, cache->bitmap_size)) {
        list_remove_entry(entry);
    }

    spinlock_unlock(&cache->lock);

    return (void*)ADDPTRS(page, first_bit * cache->size);
}

void mem_cache_free(void *ptr) {
    struct kpage *kpage;

    kpage = kpage_lookup((void*)((long)ptr & PAGE_MASK));
    spinlock_lock(&kpage->lock);
    if (!(kpage->flags & KPAGE_FLAGS_ALLOCATED)) {
        spinlock_unlock(&kpage->lock);
        return;
    }
    if (!(kpage->flags & (KPAGE_FLAGS_CACHE | KPAGE_FLAGS_CACHE_METADATA))) {
        spinlock_unlock(&kpage->lock);
        kpage->refcount--;
        if (!kpage->refcount) {
        }
    }
    spinlock_unlock(&kpage->lock);
}

int cache_init() {
    if (mem_cache_init(&mem_cache_bitmap) == -1)
        return -1;
    return 0;
}

void *page_alloc_kmap_linear(u32 npages) {
    addr_t addr;
    struct kpage *page = page_alloc(npages);
    if (!page)
        return NULL;

    addr = ADDPTRS((page-kpages) << PAGE_SHIFT, KERNEL_VIRT_ADDR);
    map_kernel_linear_with_pagetable(addr, npages << PAGE_SHIFT, PTE_PRESENT | PTE_WRITE, false);
    memset(addr, 0, PAGE_SIZE * npages);
    return (void*)addr;
}

void page_free(void *addr) {
    (void*)addr++;
}
