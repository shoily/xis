/*  File: page32.h                                                           */
/*                                                                           */
/*  Description: Header file for 32 bit page table handling code.            */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: July 9, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#ifndef PAGE32_H
#define PAGE32_H

#include "type.h"
#include "system.h"
#include "krnlconst.h"

typedef u32 pte_t;
typedef u32 pgd_t;
typedef u32 pfn_t;

#define GET_CURCPU_PGDIR ((pgd_t*)((int)&_kernel_pg_dir + (PAGE_SIZE*CUR_CPU)))

#define PGD_PRESENT   1
#define PGD_WRITE     2
#define PGD_USER      4

#define PTE_PRESENT   1
#define PTE_WRITE     2
#define PTE_USER      4

#define PAGE_MASK ~(PAGE_SIZE-1)
#define PGD_MASK ~(PGD_SIZE-1)

#define ALIGN_PGD(m) (((m)+PGD_SIZE-1)&PGD_MASK)
#define ALIGN_PAGE(m) (((m)+PAGE_SIZE-1)&PAGE_MASK)

void build_pagetable(pgd_t *pgdir, pte_t **pgtable, u32 phys_addr, u32 start, u32 length, u32 pgd_flags, u32 pte_flags);
void map_kernel_linear(pte_t **pgtable, addr_t virt_addr, u32 length, u32 pte_flags);
pfn_t page_getpfn(void *addr);
void map_kernel_linear_with_pagetable(addr_t virt_addr, u32 length, u32 pte_flags);

#endif
