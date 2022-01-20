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

    .include "krnlconst.S"

    .globl _start
_start:

    // initializing kernel page directory with zero
    movl $_kernel_pg_dir, %edi
    movl $1024, %ecx
    movl $0, %eax
1:
    stosl
    loop 1b

    // populating page directory entries for identity mapping and at 0x80000000
    movl $_kernel_pg_table_0-KERNEL_VIRT_ADDR, %edx
    leal 0x1(%edx), %eax
    movl %eax, _kernel_pg_dir-KERNEL_VIRT_ADDR
    movl %eax, _kernel_pg_dir-KERNEL_VIRT_ADDR+KERNEL_PGDIR_ENTRY

    // populating page table entries that maps 4MB spaces starting from memory address 0
    // both page directory and page table entry should be located in first 4 GB.
    cld
    movl $0x3, %eax
    movl $1024, %ecx
    movl $_kernel_pg_table_0-KERNEL_VIRT_ADDR, %edi
    
1:
    stosl
    addl $0x1000, %eax
    loop 1b

    // turning on paging
    movl $_kernel_pg_dir-KERNEL_VIRT_ADDR, %eax
    movl %eax, %cr3
    movl %cr0, %eax
    orl $0x80000000, %eax
    movl %eax, %cr0

    // jump to kernel address 0x80000000
    ljmpl $0x8, $highaddress

highaddress:

    // invalidate identity mapping
    movl $0, _kernel_pg_dir
    invlpg (0)

    // setting up kernel mode stack in high address
    movl $(_kernel_stack_0_start), %esp
    movl %esp, %ebp

    call start_kernel

loop:
    jmp loop

    // load smp binary code here
    .section mp_init, "ax"
    .global init_ap
    .align 8
init_ap:
    .incbin "mpinit.bin"

    .global init_ap_size
init_ap_size:
    .int init_ap_size - init_ap

    // load um binary code here
    .section um_init, "ax"
    .global init_um
    .align 4096
init_um:
    .incbin "um.bin"

    .global init_um_size
init_um_size:
    .int init_um_size - init_um
