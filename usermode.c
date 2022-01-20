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

char __attribute__((aligned(4096))) um_pg_table[MAX_NUM_SMPS][PAGE_SIZE];

#define USER_MODE_STACK_SIZE 8192
char __attribute__((aligned(8))) user_mode_stack[MAX_NUM_SMPS][USER_MODE_STACK_SIZE];

extern int _kernel_pg_dir;

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

void initialize_usermode() {

    pte_t *pgtable[1];

    memset(&um_pg_table[CUR_CPU][0], 0, PAGE_SIZE);
    memset(&user_mode_stack[CUR_CPU][0], 0, USER_MODE_STACK_SIZE);
    
    pgtable[0] = (pte_t*)&um_pg_table[CUR_CPU][0];

    build_pagetable((pgd_t*)((int)&_kernel_pg_dir + (PAGE_SIZE*CUR_CPU)),
					pgtable,
					(int)&init_um-KERNEL_VIRT_ADDR,
					USER_MODE_VIRT_TEXT,
					(int)init_um_size,
					PGD_PRESENT | PGD_WRITE | PGD_USER, PTE_PRESENT | PTE_WRITE | PTE_USER);

	build_pagetable((pgd_t*)((int)&_kernel_pg_dir + (PAGE_SIZE*CUR_CPU)),
					pgtable,
					(int)&user_mode_stack[CUR_CPU][0]-KERNEL_VIRT_ADDR,
					USER_MODE_VIRT_STACK,
					USER_MODE_STACK_SIZE,
					PGD_PRESENT | PGD_WRITE | PGD_USER, PTE_PRESENT | PTE_WRITE | PTE_USER);
}
