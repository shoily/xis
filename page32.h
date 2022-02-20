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

extern addr_t _kernel_pg_dir;

#define KERNEL_PGDIR_ENTRY KERNEL_PGDIR_OFFSET/sizeof(pgd_t)

#define GET_CURCPU_PGDIR ((pgd_t*)((int)&_kernel_pg_dir + (PAGE_SIZE*CUR_CPU)))
#define GET_CPU_PGDIR(cpuid) ((pgd_t*)((int)&_kernel_pg_dir + (PAGE_SIZE*(cpuid))))

#define PGD_PRESENT   1
#define PGD_WRITE     2
#define PGD_USER      4

#define PTE_PRESENT   1
#define PTE_WRITE     2
#define PTE_USER      4

#define PAGE_MASK ~(PAGE_SIZE-1)
#define PGD_MASK ~(PGD_SIZE-1)
#define PGD_FLAG_MASK 0x1FFF

#define MAP_USER 1
#define MAP_LOCAL_CPU 2
#define MAP_PGD_LOCKED 4

#define ALIGN_PGD(m) (((m)+PGD_SIZE-1)&PGD_MASK)
#define ALIGN_PAGE(m) (((m)+PAGE_SIZE-1)&PAGE_MASK)

void build_pagetable(u32 cpuid, pte_t **pgtable, addr_t phys_addr, addr_t start, u32 length, u32 pgd_flags, u32 pte_flags, u32 map_flags);
void unmap_pagetable(u32 cpuid, pte_t **pgtable, addr_t start, u32 length, u32 map_flags);
void map_kernel_with_pagetable(addr_t virt_addr, addr_t phys_addr, u32 length, u32 pte_flags, u32 map_flags);
void unmap_kernel_with_pagetable(addr_t virt_addr, u32 length, u32 map_flags);
int alloc_user_page(addr_t virt_addr, u32 length, u32 pte_flags);
pfn_t page_getpfn(void *addr);
void pgd_lock_init();
void pgd_kernel_lock_all();
void pgd_kernel_unlock_all();


#define map_kernel_linear_with_pagetable(v,l,pte_flags,map_flags) map_kernel_with_pagetable((v),(v)-KERNEL_VIRT_ADDR,(l),(pte_flags),(map_flags))

#endif
