/*****************************************************************************/
/*  File: mp_init.S                                                          */
/*                                                                           */
/*  Description: Startup code for AP.                                        */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Aug 9, 2020                                                        */
/*                                                                           */
/*****************************************************************************/

    .include "krnlconst.S"

    .code16
    .text

.globl _start
_start:

    cli
    xorw %ax, %ax
    movw %ax, %ss
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    // loads gdt
    xorw %ax, %ax
    movw %ax, %ds
    lgdt gdtdescriptor

    // turn on protected mode
    movl %cr0, %eax
    orl $1, %eax
    movl %eax, %cr0

    cli

    // jump to protected mode
    ljmpl *codesegaddr

    // protected mode code
    .code32

protectedmodeaddress:

    movl $data_seg, %eax
    movl %eax, %ds
    movl %eax, %es
    movl %eax, %ss
    movl %eax, %fs
    movl %eax, %gs


    // turning on paging
    movl AP_KERNEL_PG_DIR, %eax
    movl %eax, %cr3
    movl %cr0, %eax
    orl $0x80000000, %eax
    movl %eax, %cr0

    lock incw AP_COUNT_PHYS_ADDR+KERNEL_VIRT_ADDR

    hlt

    .code32
    .section ".high", "ax"

highaddress:

    // initialize protected mode stack
    movl $PROTECTED_MODE_STACK, %eax
    movl %eax, %esp
    movl %eax, %ebp

    hlt

    .data
    // global descriptor table (GDT)
    .p2align 5
gdt:
// offset 0x0 - NULL descriptor
	.quad 0

code_seg_addr:
// offset 0x8 - cs should point to this descriptor
	.word 0xFFFF
	.word 0
	.byte 0
	.byte 0x9A
	.byte 0xCF
	.byte 0

data_seg_addr:
// offset 0x10 - ds, ss, es, fs, and gs should point to this descriptor
	.word 0xFFFF
	.word 0
	.byte 0
	.byte 0x92
	.byte 0xCF
	.byte 0

end_of_gdt: 

codesegaddr:
    .long protectedmodeaddress
    .word code_seg

gdtdescriptor:
    .word end_of_gdt-gdt-1
    .long gdt

    .equ code_seg, code_seg_addr-gdt
    .equ data_seg, data_seg_addr-gdt
