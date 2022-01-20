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

extern addr_t _kernel_pg_dir;
extern addr_t _kernel_pg_table_0;

void build_pagetable(pgd_t *pgdir, pte_t **pgtable, u32 phys_addr, u32 start, u32 length, u32 pgd_flags, u32 pte_flags) {

    int idx = 0;
    pgd_t *pgd = pgdir + ((start >> PGD_SHIFT) & 0x3ff);
    pte_t *pte = pgtable[idx] + ((start >> PAGE_SHIFT) & 0x3ff);
    pte_t *last_pte = (pte_t*)(((long)pte + PAGE_SIZE) & PAGE_MASK);
    u32 end = start + length;

    if (!*pgd) {
        *pgd = ((long)pgtable[idx] - KERNEL_VIRT_ADDR) | pgd_flags;
    }

    while(start < end) {

        if (pte == last_pte) {
            
            idx++;
            pte = pgtable[idx];
            last_pte = pte + (PAGE_SIZE/sizeof(pte_t));
            pgd++;

            if (!*pgd) {
                *pgd = (((long)pgtable[idx] - KERNEL_VIRT_ADDR) | pgd_flags);
            } else if ((*pgd & PAGE_MASK) != pgd_flags) {
                *pgd |= pgd_flags;
            }
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

    end_virt_addr = ALIGN_PAGE(virt_addr + length);
    virt_addr = virt_addr & PAGE_MASK;

    pte = (pte_t*)ADDPTRS(&_kernel_pg_table_0, (((VIRT_TO_PHYS_ADDR_LINEAR(virt_addr) >> PGD_SHIFT) & 0x3ff) << PAGE_SHIFT));
    pte += (((long)(virt_addr) >> PAGE_SHIFT) & 0x3ff);

    while (virt_addr != end_virt_addr) {
        pgtbl[0] = (pte_t*)((long)pte & PAGE_MASK);
        build_pagetable(GET_CURCPU_PGDIR, pgtbl, virt_addr - KERNEL_VIRT_ADDR, virt_addr, PAGE_SIZE, PGD_PRESENT | PGD_WRITE, pte_flags);
        virt_addr = ADDPTRS(virt_addr, PAGE_SIZE);
        pte++;
    }
}

void map_kernel_linear(pte_t **pgtable, addr_t virt_addr, u32 length, u32 pte_flags) {
    build_pagetable(GET_CURCPU_PGDIR, pgtable, virt_addr - KERNEL_VIRT_ADDR, virt_addr, length, PGD_PRESENT | PGD_WRITE, pte_flags);
}

pfn_t page_getpfn(void *addr) {
    pgd_t *pgd = GET_CURCPU_PGDIR + (((long)(addr) >> PGD_SHIFT) & 0x3ff);
    pte_t *pte = ((pte_t*)ADDPTRS(*pgd, KERNEL_VIRT_ADDR)) + (((long)(addr) >> PAGE_SHIFT) & 0x3ff);
    return (*pte & PAGE_MASK);
}
