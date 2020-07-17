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

void build_pagetable(pgd_t *pgdir, pte_t **pgtable, int phys_addr, int start, int length, int pgd_flags, int pte_flags) {

    int idx = 0;
    pgd_t *pgd = pgdir + ((start >> 22) & 0x3ff);
    pte_t *pte = pgtable[idx] + ((start >> 12) & 0x3ff);    
    pte_t *last_pte = (pte_t*)(((int)pte + PAGE_SIZE) & ~(PAGE_SIZE-1));
    int end = start + length;

    if (!*pgd) {
        *pgd = ((int)pgtable[idx] - KERNEL_VIRT_ADDR) | pgd_flags;
    }

    while(start < end) {

        if (pte == last_pte) {
            
            idx++;
            pte = pgtable[idx];
            last_pte = pte + (PAGE_SIZE/sizeof(pte_t));
            pgd++;

            if (!*pgd) {
                *pgd = (((int)pgtable[idx] - KERNEL_VIRT_ADDR) | pgd_flags);
            }
        }

        *pte = (phys_addr | pte_flags);
        
        pte++;
        start += 0x1000;
        phys_addr += 0x1000;
    }
}
