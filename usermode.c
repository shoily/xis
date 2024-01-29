/*****************************************************************************/
/*  File: usermode.c                                                         */
/*                                                                           */
/*  Description: Source file for user mode process support.                  */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: July 7, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#include "usermode.h"
#include "util.h"
#include "system.h"
#include "page32.h"
#include "setup32.h"
#include "system.h"
#include "debug.h"
#include "memory.h"
#include "page32.h"

//char __attribute__((aligned(4096))) um_pg_table[MAX_NUM_SMPS][PAGE_SIZE];

#define USER_MODE_STACK_SIZE 8192
//char __attribute__((aligned(8))) user_mode_stack[MAX_NUM_SMPS][USER_MODE_STACK_SIZE];

extern int init_um;
extern int init_um_size;

void switch_to_um() {

    __asm__ __volatile__("cli;"
                         "movl %0, %%ds;"
                         "movl %0, %%es;"
                         "movl %0, %%fs;"
                         "movl %0, %%gs;"
                         "pushl %1;"           // ss
                         "pushl %2;"           // esp
                         "pushf;"              // eflags
                         "popl %%eax;"         // pop eflags to modify
                         "orl $0x200, %%eax;"  // turn on IF
                         "pushl %%eax;"        // push for eflags
                         "pushl %3;"           // cs
                         "pushl %4;"           // ip
                         "mfence;"
                         "iret;"
                         :
                         : "q" (USER_DATA_SEG | 3), "i"(USER_DATA_SEG | 3), "i" (USER_MODE_VIRT_STACK + USER_MODE_STACK_SIZE), "i" (USER_CODE_SEG | 3), "i" (USER_MODE_VIRT_TEXT)
                         : "%eax"
                         );
}

void load_usermode_text() {
    pgd_t *pgd;
    pte_t *pte;
    addr_t addr;
    int chunk;
    char *s = (char*)&init_um;
    int length = init_um_size;

    pgd = GET_CPU_PGDIR(CUR_CPU) + ((USER_MODE_VIRT_TEXT >> PGD_SHIFT) & 0x3ff);
    pte = (pte_t*)ADDPTRS(*pgd & PAGE_MASK, KERNEL_VIRT_ADDR);
    pte += ((USER_MODE_VIRT_TEXT >> PAGE_SHIFT) & 0x3ff);

    while(length) {
        addr = (addr_t)ADDPTRS(*pte & PAGE_MASK, KERNEL_VIRT_ADDR);
        map_kernel_linear_with_pagetable(addr,
                                         PAGE_SIZE,
                                         PTE_WRITE,
                                         MAP_LOCAL_CPU);
        chunk = length > PAGE_SIZE ? PAGE_SIZE : length;
        memcpy(addr, s, (unsigned int)chunk);
        unmap_kernel_with_pagetable(addr, PAGE_SIZE, MAP_LOCAL_CPU);
        s += chunk;
        length -= chunk;
        pte++;
        if (pte == (pte_t*)ALIGN_PAGE((addr_t)pte)) {
            pgd++;
            pte = (pte_t*)ADDPTRS(*pgd & PAGE_MASK, KERNEL_VIRT_ADDR);
        }
    }
}

void initialize_usermode() {
    int err;
    u32 pgd = (u32)&_kernel_pg_dir + (CUR_CPU*PAGE_SIZE);

    memset((void*)pgd, 0, KERNEL_PGDIR_OFFSET);

    if ((err = alloc_user_page(USER_MODE_VIRT_STACK, USER_MODE_STACK_SIZE, PTE_WRITE)) != 0) {
        printf(KERNEL_ERR, "alloc_user_page failed for stack: %d %d", err, CUR_CPU);
        return;
    }

    if ((err = alloc_user_page(USER_MODE_VIRT_TEXT, ALIGN_PAGE(init_um_size), 0)) != 0) {
        printf(KERNEL_ERR, "alloc_user_page failed for text section: %d ", err);
        page_free_user((void *)USER_MODE_VIRT_STACK);
        return;
    }

    load_usermode_text();
}
