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

    // jump to protected mode
    ljmpl *protectmodecodesegaddr

    // protected mode code
    .code32

protectedmodeaddress:

    movl $data_seg, %eax
    movl %eax, %ds
    movl %eax, %es
    movl %eax, %ss
    movl %eax, %fs
    movl %eax, %gs

    // get LAPIC id
    movl AP_LAPIC_BASE_REGISTER, %ebx
    addl $0x20, %ebx
    movl (%ebx), %eax
    shrl $24, %eax
    movl %eax, %ecx

    // calculate start of kernel mode stack for this processor
    movl $KERNEL_STACK_SIZE, %ebx
    mull %ebx
    movl AP_FIRST_STACK, %ebx
    addl %ebx, %eax

    // initialize protected mode stack
    movl %eax, %esp
    movl %esp, %ebp

    // turning on paging
    movl %ecx, %eax
    movl $PAGE_SIZE, %ebx
    mull %ebx
    movl AP_KERNEL_PG_DIR, %ebx
    addl %ebx, %eax
    movl %eax, %cr3

    movl %cr0, %ebx
    orl $0x80000000, %ebx
    movl %ebx, %cr0

    ljmpl *highcodesegaddr

    .code32
    .section ".high", "ax"

highaddress:

    // eax contains PGD
    // clear TLB entries for 0x7C00
    invlpg (0x7000)

    // clear identity mapping for first 4MB
    //addl $KERNEL_VIRT_ADDR, %eax
    leal KERNEL_VIRT_ADDR(%eax), %ebx
    movl $0, (%ebx)

    // push lapic id in the stack
    pushl %ecx
    movl AP_FINISH_CODE+KERNEL_VIRT_ADDR, %eax

    // call finish_mp_initialization
    call *%eax

loop:
    jmp loop

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

protectmodecodesegaddr:
    .long protectedmodeaddress
    .word code_seg

highcodesegaddr:
    .long highaddress
    .word code_seg

gdtdescriptor:
    .word end_of_gdt-gdt-1
    .long gdt

    .equ code_seg, code_seg_addr-gdt
    .equ data_seg, data_seg_addr-gdt
