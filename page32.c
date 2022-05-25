/*****************************************************************************/
/*  File: page32.c                                                           */
/*                                                                           */
/*  Description: Source file for 32 bit page table handling code.            */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: July 9, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#include "page32.h"
#include "system.h"
#include "lock.h"
#include "memory.h"
#include "debug.h"

extern addr_t _kernel_pg_table_0;
extern spinlock spinlock_smp;
extern int smp_bits;
extern int smp_on_hold;
extern pgd_t _master_kernel_pg_dir;

// locks for each GDT entry in kernel space shared among all the CPUs.
spinlock lock_pgd_kernel[(PAGE_SIZE-KERNEL_PGDIR_OFFSET)/sizeof(pgd_t)];

// locks in userspace. One lock per CPU for all GDT entries .
spinlock lock_pgd_user[MAX_NUM_SMPS];

#define LOCK_PGD(cpuid, addr, flags) {                                  \
        if (!((flags) & (MAP_USER | MAP_LOCAL_CPU)) &&                  \
            (addr) >= (addr_t)KERNEL_VIRT_ADDR) {                       \
            spinlock_lock(&lock_pgd_kernel[((((addr)-KERNEL_VIRT_ADDR) >> PGD_SHIFT) & 0x3ff)]); \
        } else {                                                        \
            spinlock_lock(&lock_pgd_user[(cpuid)]);                     \
        }                                                               \
    }
#define UNLOCK_PGD(cpuid, addr, flags) {                                \
        if (!((flags) & (MAP_USER | MAP_LOCAL_CPU)) &&                  \
            (addr) >= (addr_t)KERNEL_VIRT_ADDR) {                       \
            spinlock_unlock(&lock_pgd_kernel[((((addr)-KERNEL_VIRT_ADDR) >> PGD_SHIFT) & 0x3ff)]); \
        } else {                                                        \
            spinlock_unlock(&lock_pgd_user[(cpuid)]);                   \
        }                                                               \
    }

void build_pagetable(u32 cpuid, pte_t **pgtable, addr_t phys_addr, addr_t start, u32 length, u32 pgd_flags, u32 pte_flags, u32 map_flags) {
    int idx = 0;
    bool locked = map_flags & MAP_PGD_LOCKED;
    pgd_t *pgdir = (map_flags & (MAP_USER | MAP_LOCAL_CPU)) ? GET_CPU_PGDIR(cpuid) : &_master_kernel_pg_dir;
    pgd_t *pgd = pgdir + ((start >> PGD_SHIFT) & 0x3ff);
    pte_t *pte = pgtable[idx] + ((start >> PAGE_SHIFT) & 0x3ff);
    pte_t *last_pte = (pte_t*)(((long)pte + PAGE_SIZE) & PAGE_MASK);
    addr_t end = start + length;

    if (!locked)
        LOCK_PGD(cpuid, start, map_flags);
    if (!*pgd) {
        *pgd = ((long)pgtable[idx] - KERNEL_VIRT_ADDR) | pgd_flags;
    } else if (((*pgd & PGD_FLAG_MASK) ^ pgd_flags) != 0) {
        *pgd |= pgd_flags;
    }
    if (!locked)
        UNLOCK_PGD(cpuid, start, map_flags);

    while (start < end) {
        if (pte == last_pte) {
            idx++;
            pte = pgtable[idx];
            last_pte = (pte_t*)(((long)pte + PAGE_SIZE) & PAGE_MASK);
            pgd++;

            if (!locked)
                LOCK_PGD(cpuid, start, map_flags);
            if (!*pgd) {
                *pgd = (((long)pgtable[idx] - KERNEL_VIRT_ADDR) | pgd_flags);
            } else if (((*pgd & PGD_FLAG_MASK) ^ pgd_flags) != 0) {
                *pgd |= pgd_flags;
            }
            if (!locked)
                UNLOCK_PGD(cpuid, start, map_flags);
        }

        *pte = (phys_addr | pte_flags);
        pte++;
        start += PAGE_SIZE;
        phys_addr += PAGE_SIZE;
    }
}

void unmap_pagetable(u32 cpuid, pte_t **pgtable, addr_t start, u32 length, u32 map_flags) {
    int idx = 0;
    bool locked = map_flags & MAP_PGD_LOCKED;
    pgd_t *pgdir = (map_flags & (MAP_USER | MAP_LOCAL_CPU)) ? GET_CPU_PGDIR(cpuid) : &_master_kernel_pg_dir;;
    pgd_t *pgd = pgdir + ((start >> PGD_SHIFT) & 0x3ff);
    pte_t *pte = pgtable[idx] + ((start >> PAGE_SHIFT) & 0x3ff);
    pte_t *last_pte = (pte_t*)(((long)pte + PAGE_SIZE) & PAGE_MASK);
    addr_t end = start + length;

    if (!locked)
        LOCK_PGD(cpuid, start, map_flags);
    if (!*pgd) {
        start += ((last_pte-pte) << PAGE_SHIFT);
        pte = last_pte;
    }
    if (!locked)
        UNLOCK_PGD(cpuid, start, map_flags);

    while (start < end) {
        if (pte == last_pte) {
            idx++;
            pte = pgtable[idx];
            last_pte = pte + (PAGE_SIZE/sizeof(pte_t));
            pgd++;

            if (!locked)
                LOCK_PGD(cpuid, start + PAGE_SIZE, map_flags);
            if (!*pgd) {
                pte = last_pte;
                if (!locked)
                    UNLOCK_PGD(cpuid, start + PAGE_SIZE, map_flags);
                start += PGD_SIZE;
                continue;
            }
            if (!locked)
                UNLOCK_PGD(cpuid, start + PAGE_SIZE, map_flags);
        }

        *pte = 0;

        pte++;
        start += PAGE_SIZE;
    }
}

void map_kernel_with_pagetable(addr_t virt_addr, addr_t phys_addr, u32 length, u32 pte_flags, u32 map_flags) {
    pte_t *pgtbl[1];
    pte_t *pte;
    addr_t end_virt_addr;

    if ((virt_addr < (addr_t)KERNEL_VIRT_ADDR) || (map_flags & MAP_USER))
        return;

    end_virt_addr = ALIGN_PAGE(virt_addr + length);
    virt_addr = virt_addr & PAGE_MASK;

    pte = (pte_t*)ADDPTRS(&_kernel_pg_table_0, (((VIRT_TO_PHYS_ADDR_LINEAR(virt_addr) >> PGD_SHIFT) & 0x3ff) << PAGE_SHIFT));
    pte += (((long)(virt_addr) >> PAGE_SHIFT) & 0x3ff);

    while (virt_addr != end_virt_addr) {
        pgtbl[0] = (pte_t*)((long)pte & PAGE_MASK);
        build_pagetable(CUR_CPU,
                        pgtbl,
                        phys_addr,
                        virt_addr,
                        PAGE_SIZE,
                        PGD_PRESENT | PGD_WRITE,
                        pte_flags | PTE_PRESENT,
                        map_flags);

        virt_addr = ADDPTRS(virt_addr, PAGE_SIZE);
        phys_addr = ADDPTRS(phys_addr, PAGE_SIZE);
        pte++;
    }
}

void unmap_kernel_with_pagetable(addr_t virt_addr, u32 length, u32 map_flags) {
    pte_t *pgtbl[1];
    pte_t *pte;
    addr_t end_virt_addr;

    if ((virt_addr < (addr_t)KERNEL_VIRT_ADDR) || (map_flags & MAP_USER))
        return;

    end_virt_addr = ALIGN_PAGE(virt_addr + length);
    virt_addr = virt_addr & PAGE_MASK;

    pte = (pte_t*)ADDPTRS(&_kernel_pg_table_0, (((VIRT_TO_PHYS_ADDR_LINEAR(virt_addr) >> PGD_SHIFT) & 0x3ff) << PAGE_SHIFT));
    pte += (((long)(virt_addr) >> PAGE_SHIFT) & 0x3ff);

    while (virt_addr != end_virt_addr) {
        pgtbl[0] = (pte_t*)((long)pte & PAGE_MASK);
        unmap_pagetable(CUR_CPU, pgtbl, virt_addr, PAGE_SIZE, map_flags);
        virt_addr = ADDPTRS(virt_addr, PAGE_SIZE);
        pte++;
    }
}

int alloc_user_page(addr_t virt_addr, u32 length, u32 pte_flags) {
    u32 pgtbl_idx;
    addr_t addr, phys_addr;
    struct kpage *page_buf;
    struct kpage *page_pgtbl = NULL;
    pte_t *pgtbl[1];
    pgd_t *pgd;
    u32 nr_pgtbls = 0, prev_nr_pgtbls = 0;
    u32 pgd_len = ALIGN_PGD(length);
    pgd_t *pgdir = GET_CPU_PGDIR(CUR_CPU);

    if (!virt_addr)
        return ERR_NOT_SUPPORTED;

    length = ALIGN_PAGE(length);

    if (virt_addr >= (addr_t)KERNEL_VIRT_ADDR)
        return ERR_INVALID;

    page_buf = page_alloc(ALIGN_PAGE(length) >> PAGE_SHIFT);
    if (!page_buf)
        return ERR_NOMEM;

    prev_nr_pgtbls = 0;

retry_pgtbl_read:
    LOCK_PGD(CUR_CPU, 0, MAP_USER | MAP_LOCAL_CPU);
    addr = virt_addr;
    while (pgd_len) {
        pgd = pgdir + ((addr >> PGD_SHIFT) & 0x3ff);
        if (!*pgd)
            nr_pgtbls++;
        pgd_len -= PGD_SIZE;
        addr = ADDPTRS(addr, PGD_SIZE);
    }

    if (prev_nr_pgtbls != nr_pgtbls) {
        UNLOCK_PGD(CUR_CPU, addr, MAP_USER | MAP_LOCAL_CPU);
        if (page_pgtbl) {
            page_free(page_pgtbl);
            page_pgtbl = NULL;
        }
        if (nr_pgtbls) {
            page_pgtbl = page_alloc(nr_pgtbls);
            if (!page_pgtbl) {
                UNLOCK_PGD(CUR_CPU, addr, MAP_USER | MAP_LOCAL_CPU);
                page_free(page_buf);
                return ERR_NOMEM;
            }
        }
        prev_nr_pgtbls = nr_pgtbls;
        goto retry_pgtbl_read;
    }

    spinlock_lock(&spinlock_smp);
    smp_on_hold++;
    spinlock_unlock(&spinlock_smp);

    if (nr_pgtbls)
        map_kernel_linear_with_pagetable((addr_t)ADDPTRS(KPAGE_TO_PHYS_ADDR(page_pgtbl), KERNEL_VIRT_ADDR),
                                         nr_pgtbls << PAGE_SHIFT,
                                         PTE_WRITE,
                                         MAP_LOCAL_CPU | MAP_PGD_LOCKED);

    phys_addr = (addr_t)KPAGE_TO_PHYS_ADDR(page_buf);
    pgtbl_idx = 0;
    while (length) {
        pgd = pgdir + ((virt_addr >> PGD_SHIFT) & 0x3ff);
        if (*pgd) {
            pgtbl[0] = (pte_t*)ADDPTRS((*pgd & PAGE_MASK), KERNEL_VIRT_ADDR);
        } else {
            pgtbl[0] = (pte_t*)ADDPTRS(ADDPTRS(KPAGE_TO_PHYS_ADDR(page_pgtbl), pgtbl_idx << PAGE_SHIFT), KERNEL_VIRT_ADDR);
            pgtbl_idx++;
        }

        build_pagetable(CUR_CPU,
                        pgtbl,
                        phys_addr,
                        virt_addr,
                        PAGE_SIZE,
                        PGD_PRESENT | PGD_USER | ((pte_flags & PTE_WRITE) ? PGD_WRITE : 0),
                        PTE_PRESENT | PTE_USER | pte_flags,
                        MAP_USER | MAP_PGD_LOCKED);

        virt_addr = ADDPTRS(virt_addr, PAGE_SIZE);
        phys_addr = ADDPTRS(phys_addr, PAGE_SIZE);
        length -= PAGE_SIZE;
    }

    if (nr_pgtbls)
        unmap_kernel_with_pagetable((addr_t)ADDPTRS(KPAGE_TO_PHYS_ADDR(page_pgtbl), KERNEL_VIRT_ADDR),
                                   nr_pgtbls << PAGE_SHIFT,
                                   MAP_LOCAL_CPU | MAP_PGD_LOCKED);

    UNLOCK_PGD(CUR_CPU, 0, MAP_USER | MAP_LOCAL_CPU);

    spinlock_lock(&spinlock_smp);
    smp_on_hold--;
    spinlock_unlock(&spinlock_smp);

    return 0;
}

pfn_t page_getpfn(void *addr) {
    pgd_t *pgd = GET_CURCPU_PGDIR + (((long)(addr) >> PGD_SHIFT) & 0x3ff);
    pte_t *pte = ((pte_t*)ADDPTRS(*pgd, KERNEL_VIRT_ADDR)) + (((long)(addr) >> PAGE_SHIFT) & 0x3ff);
    return (*pte & PAGE_MASK);
}

void pgd_lock_init() {
    u32 i;
    for(i = 0; i < (sizeof(lock_pgd_kernel)/sizeof(spinlock)); i++)
        INIT_SPIN_LOCK(&lock_pgd_kernel[i]);
    for(i = 0; i < MAX_NUM_SMPS; i++)
        INIT_SPIN_LOCK(&lock_pgd_user[i]);
}

void pgd_kernel_lock_all() {
    u32 i;
    for(i=0;i<sizeof(lock_pgd_kernel)/sizeof(spinlock);i++)
        spinlock_lock(&lock_pgd_kernel[i]);
}

void pgd_kernel_unlock_all() {
    u32 i;
    for(i=0;i<sizeof(lock_pgd_kernel)/sizeof(spinlock);i++)
        spinlock_unlock(&lock_pgd_kernel[i]);
}
void pagetable_delete_entry(u32 pfn, u32 pages) {

    for (u32 i = 0; i < pages; i++, pfn++) {
        //clear_tlb(pfn);
    }
}
