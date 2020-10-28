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

typedef int pte_t;
typedef int pgd_t;

#define PGD_PRESENT   1
#define PGD_WRITE     2
#define PGD_USER      4

#define PTE_PRESENT   1
#define PTE_WRITE     2
#define PTE_USER      4

void build_pagetable(pgd_t *pgdir, pte_t **pgtable, int phys_addr, int start, int length, int pgd_flags, int pte_flags);

#endif
