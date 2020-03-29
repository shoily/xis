/*****************************************************************************/
/*  File: x86_32.c                                                           */
/*                                                                           */
/*  Description: x86 32 bit GDT re-initialization code to enable IDT, LDT    */
/*  and TSS                                                                  */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Mar 25, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#include "common.h"
#include "x86_32.h"

struct gdt_entry __attribute__((aligned(8))) gdt[LAST_SEG*8];

struct _gdt_desc {
    unsigned short size;
    struct gdt_entry *gdt;
}__attribute__((packed));

struct _gdt_desc __attribute__((aligned(8))) gdt_desc;

//
// Re-initializes GDT for kernel code and data segement selectors
//
void start32() {

    memset(gdt, sizeof(gdt), 0);
    
    gdt_desc.size = sizeof(gdt) - 1;
    gdt_desc.gdt = gdt;

    SET_KERNEL_CODE_SEGMENT(gdt, 0xfffff, 0);
    SET_KERNEL_DATA_SEGMENT(gdt, 0xfffff, 0);
    
    SET_USER_CODE_SEGMENT(gdt, 0xfffff, 0);
    SET_USER_DATA_SEGMENT(gdt, 0xfffff, 0);
    
    __asm__ __volatile__("movl $%0, %%eax;"
                         "lgdt (%%eax);"
                         "ljmpl %1, $reinitsegs;"
                         "reinitsegs:;"
                         "movl %2, %%eax;"
                         "movl %%eax, %%ds;"
                         "movl %%eax, %%es;"
                         "movl %%eax, %%ss;"
                         "movl %%eax, %%fs;"
                         "movl %%eax, %%gs;"
                         :
                         : "m" (gdt_desc), "N" (KERNEL_CODE_SEG), "N" (KERNEL_DATA_SEG)
                         : "%eax"
                         );
}
