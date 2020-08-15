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

char __attribute__((aligned(4096))) um_pg_table[4096];
char __attribute__((aligned(4096))) user_mode_program[4096];

#define USER_MODE_STACK_SIZE 8192
char __attribute__((aligned(8))) user_mode_stack[USER_MODE_STACK_SIZE];

extern int um, um_size;
extern int _kernel_pg_dir;

__attribute__((section("um"))) __attribute__((aligned(4096))) void test_usermode_func() {
    int *a = (int*)((char*) USER_MODE_VIRT_TEXT + 0x100);
    int i = 1;

    __asm__ __volatile__("int $128;" : : :);
    *a = i++;
    
    __asm__ __volatile("movl $0x10000016, %%ecx; jmp *%%ecx;" : : : "%ecx");
}

void load_usermode_function(char *usermodefunction) {

    char *s = usermodefunction;
    char *d = user_mode_program;
    int i;

    memset(user_mode_program, sizeof(user_mode_program), 0);

    for(i=(int)&um;i<(int)&um+(int)&um_size;i++) {
        *d++ = *s++;
    }
}

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

    memset(um_pg_table, sizeof(um_pg_table), 0);
    memset(user_mode_stack, sizeof(user_mode_stack), 0);
    
    pgtable[0] = (pte_t*)um_pg_table;
    
    load_usermode_function((char*)test_usermode_func);
    
    build_pagetable((pgd_t*)&_kernel_pg_dir, pgtable, (int)user_mode_program-KERNEL_VIRT_ADDR, USER_MODE_VIRT_TEXT, (int)&um_size, PGD_PRESENT | PGD_WRITE | PGD_USER, PTE_PRESENT | PTE_WRITE | PTE_USER);
    build_pagetable((pgd_t*)&_kernel_pg_dir, pgtable, (int)user_mode_stack-KERNEL_VIRT_ADDR, USER_MODE_VIRT_STACK, USER_MODE_STACK_SIZE, PGD_PRESENT | PGD_WRITE | PGD_USER, PTE_PRESENT | PTE_WRITE | PTE_USER);

    MFENCE;
    
    switch_to_um();
}
