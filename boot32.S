/*****************************************************************************/
/*  File: boot32.S                                                           */
/*                                                                           */
/*  Description: This is the boot code for kernel.                           */
/*  It should be written at sector 1 (second sector). Uses boot32.ld.        */
/*  It turns on paging and maps the first 4MB of kernel at 0x800000000       */
/*  virtual memory location. It then calls start_kernel routine from start.c */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Feb 11, 2020                                                       */
/*                                                                           */
/*****************************************************************************/
    
    .code32
    .text

    .equ KERNEL_VIRT_ADDR, 0x80000000
    .equ KERNEL_PGDIR_ENTRY, (KERNEL_VIRT_ADDR >> 20)
    .equ STACK_SIZE, 0x1000

    .globl boot_pg_table
    .globl _start
_start:

    // populating page directory entries for identity mapping and at 0x80000000
    movl $boot_pg_table-KERNEL_VIRT_ADDR, %edx
    leal 0x1(%edx), %eax
    movl %eax, boot_page_directory-KERNEL_VIRT_ADDR
    movl %eax, boot_page_directory-KERNEL_VIRT_ADDR+KERNEL_PGDIR_ENTRY

    // populating page table entries that maps 4MB spaces starting from memory address 0
    cld
    movl $0x3, %eax
    movl $1024, %ecx
    movl $boot_pg_table-KERNEL_VIRT_ADDR, %edi
    
1:
    stosl
    addl $0x1000, %eax
    loop 1b

    // turning on paging
    movl $boot_page_directory-KERNEL_VIRT_ADDR, %eax
    movl %eax, %cr3
    movl %cr0, %eax
    orl $0x80000000, %eax
    movl %eax, %cr0

    // jump to kernel address 0x80000000
    ljmpl $0x8, $highaddress

highaddress:

    // invalidate identity mapping
    movl $0, boot_page_directory
    invlpg (0)

    // setting up kernel mode stack in high address
    movl $(stack+STACK_SIZE), %esp
    movl %esp, %ebp

    // call start_kernel routine
    pushl %ebp
    call start_kernel

loop:
    jmp loop

    .data
    .p2align 12

    // page table for kernel
boot_pg_table:
    .fill 1024, 4, 0

    // page directory
boot_page_directory:
    .fill 1024, 4, 0

    .bss
    .p2align 5

    // stack in high address
    .lcomm stack, STACK_SIZE
