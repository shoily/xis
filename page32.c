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

extern addr_t _kernel_pg_table_0;
extern spinlock spinlock_smp;
extern int smp_bits;
extern int smp_on_hold;

spinlock lock_pgd[(PAGE_SIZE-KERNEL_PGDIR_OFFSET)/sizeof(pgd_t)];

void build_pagetable(u32 cpuid, pte_t **pgtable, u32 phys_addr, u32 start, u32 length, u32 pgd_flags, u32 pte_flags) {

    int idx = 0;
    pgd_t *pgdir = GET_CPU_PGDIR(cpuid);
    pgd_t *pgd = pgdir + ((start >> PGD_SHIFT) & 0x3ff);
    pte_t *pte = pgtable[idx] + ((start >> PAGE_SHIFT) & 0x3ff);
    pte_t *last_pte = (pte_t*)(((long)pte + PAGE_SIZE) & PAGE_MASK);
    u32 end = start + length;

    spinlock_lock(&lock_pgd[pgd-pgdir]);
    if (!*pgd) {
        *pgd = ((long)pgtable[idx] - KERNEL_VIRT_ADDR) | pgd_flags;
    } else if (((*pgd & PGD_FLAG_MASK) ^ pgd_flags) != 0) {
        *pgd |= pgd_flags;
    }
    spinlock_unlock(&lock_pgd[pgd-pgdir]);

    while(start < end) {

        if (pte == last_pte) {
            
            idx++;
            pte = pgtable[idx];
            last_pte = pte + (PAGE_SIZE/sizeof(pte_t));
            pgd++;

            spinlock_lock(&lock_pgd[pgd-pgdir]);
            if (!*pgd) {
                *pgd = (((long)pgtable[idx] - KERNEL_VIRT_ADDR) | pgd_flags);
            } else if (((*pgd & PGD_FLAG_MASK) ^ pgd_flags) != 0) {
                *pgd |= pgd_flags;
            }
            spinlock_unlock(&lock_pgd[pgd-pgdir]);
        }

        *pte = (phys_addr | pte_flags);
        
        pte++;
        start += PAGE_SIZE;
        phys_addr += PAGE_SIZE;
    }
}

void map_kernel_linear_with_pagetable(addr_t virt_addr, u32 length, u32 pte_flags) {
    pte_t *pgtbl[1];
    pte_t *pte;
    addr_t end_virt_addr;
    int i;
    u32 local_smp_bits;
    bool first_core;

    end_virt_addr = ALIGN_PAGE(virt_addr + length);
    virt_addr = virt_addr & PAGE_MASK;

    pte = (pte_t*)ADDPTRS(&_kernel_pg_table_0, (((VIRT_TO_PHYS_ADDR_LINEAR(virt_addr) >> PGD_SHIFT) & 0x3ff) << PAGE_SHIFT));
    pte += (((long)(virt_addr) >> PAGE_SHIFT) & 0x3ff);

    while (virt_addr != end_virt_addr) {
        first_core = false;
        spinlock_lock(&spinlock_smp);
        local_smp_bits = smp_bits;
        if (virt_addr != end_virt_addr)
            smp_on_hold++;
        spinlock_unlock(&spinlock_smp);
        pgtbl[0] = (pte_t*)((long)pte & PAGE_MASK);
        for (i = 0; i < MAX_NUM_SMPS; i++) {
            if ((1 << i) & local_smp_bits) {
                build_pagetable(i, pgtbl, virt_addr - KERNEL_VIRT_ADDR, virt_addr, PAGE_SIZE, PGD_PRESENT | PGD_WRITE, pte_flags);
                if (!first_core) {
                    first_core = true;
                    spinlock_lock(&spinlock_smp);
                    smp_on_hold--;
                    spinlock_unlock(&spinlock_smp);
                }
            }
        }
        virt_addr = ADDPTRS(virt_addr, PAGE_SIZE);
        pte++;
    }
}

pfn_t page_getpfn(void *addr) {
    pgd_t *pgd = GET_CURCPU_PGDIR + (((long)(addr) >> PGD_SHIFT) & 0x3ff);
    pte_t *pte = ((pte_t*)ADDPTRS(*pgd, KERNEL_VIRT_ADDR)) + (((long)(addr) >> PAGE_SHIFT) & 0x3ff);
    return (*pte & PAGE_MASK);
}

void pgd_lock_init() {
    u32 i;
    for(i = 0; i < (sizeof(lock_pgd)/sizeof(spinlock)); i++)
        INIT_SPIN_LOCK(&lock_pgd[i]);
}

void pgd_lock_all() {
    u32 i;
    for(i=0;i<sizeof(lock_pgd)/sizeof(spinlock);i++)
        spinlock_lock(&lock_pgd[i]);
}

void pgd_unlock_all() {
    u32 i;
    for(i=0;i<sizeof(lock_pgd)/sizeof(spinlock);i++)
        spinlock_unlock(&lock_pgd[i]);
}
