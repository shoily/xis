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
    .globl _end_kernel_init_text
_start:

    // initializing master kernel page directory with zero
    cld
    movl $_master_kernel_pg_dir-KERNEL_VIRT_ADDR, %edi
    movl $1024, %ecx
    movl $0, %eax
1:
    stosl
    loop 1b

    // aligns end of pgds with PGD_SIZE (currently 4MB)
    movl $_kernel_pg_table_0-KERNEL_VIRT_ADDR, %eax
    addl $~PGD_MASK, %eax
    andl $PGD_MASK, %eax

    // page tables pages in bytes
    shrl $(PGD_SHIFT-PAGE_SHIFT), %eax

    // end of page tables
    addl $_kernel_pg_table_0-KERNEL_VIRT_ADDR, %eax
    movl %eax, %ebx

    // aligns end of page tables with PGD_SIZE (currently 4MB)
    addl $~PGD_MASK, %eax
    andl $PGD_MASK, %eax

    testl %eax, %ebx
    jnz 1f

    // we fit all mapping of kernel code, page directories
    // and page tables using our calculated end of page tables.
    // But we will not be able to map more page tables because we
    // have exhausted all the entries in the last page table about to be
    // mapped. So the solution is to add one more page table. Now, we can extend
    // more page tables because the first entry of the new page table maps itself.
    addl $PAGE_SIZE, %ebx
    movl %ebx, %eax
    addl $~PGD_MASK, %eax
    andl $PGD_MASK, %eax
1:
    // eax stores end of page table aligned with PGD_SIZE to
    // populate page directory entries.
    // ebx stores end of page table
    movl %ebx, _end_kernel_mapped_memory-KERNEL_VIRT_ADDR

    // populating page directory entries for both identity mapping and at 0x80000000
    movl %eax, %ecx
    shrl $PGD_SHIFT, %ecx
    movl $_kernel_pg_table_0-KERNEL_VIRT_ADDR, %edx
    leal 0x1(%edx), %eax
    movl $_master_kernel_pg_dir-KERNEL_VIRT_ADDR, %edi
    movl $_master_kernel_pg_dir-KERNEL_VIRT_ADDR+KERNEL_PGDIR_OFFSET, %edx
1:
    movl %eax, (%edx)
    movl %eax, (%edi)
    addl $4, %edx
    addl $4, %edi
    addl $PAGE_SIZE, %eax
    loop 1b

    // populating page table entries
    cld
    movl $0x3, %eax
    movl %ebx, %ecx
    shrl $PAGE_SHIFT, %ecx
    movl $_kernel_pg_table_0-KERNEL_VIRT_ADDR, %edi
1:
    stosl
    addl $0x1000, %eax
    loop 1b

    // turning on paging
    movl $_master_kernel_pg_dir-KERNEL_VIRT_ADDR, %eax
    movl %eax, %cr3
    movl %cr0, %eax
    orl $0x80000000, %eax
    movl %eax, %cr0

    // jump to kernel address 0x80000000
    ljmpl $0x8, $highaddress

highaddress:

    // invalidate TLB for identity mapping
    movl %ebx, %ecx
    shrl $PAGE_SHIFT, %ecx
    xorl %eax, %eax

1:
    invlpg (%eax)
    addl $PAGE_SIZE, %eax
    loop 1b

    // invalidate identity mapping
    movl %ebx, %ecx
    addl $~PGD_MASK, %ecx
    andl $PGD_MASK, %ecx
    shrl $PGD_SHIFT, %ecx

    xorl %eax, %eax
    movl $_master_kernel_pg_dir, %edi
    cld
1:
    stosl
    loop 1b

    // APIC ID 0 to gs
    movl %eax, %gs

    // setting up kernel mode stack in high address
    movl $(_kernel_stack_0_start), %esp
    movl %esp, %ebp

    call start_kernel

    movl $9, %eax
    addl $48, %eax
    orl $(0xF << 8), %eax
    movl %eax, VIDEO_BUFFER+KERNEL_VIRT_ADDR

loop:
    jmp loop

_end_kernel_init_text:

    .data
    .global _end_kernel_mapped_memory
    .align 8
_end_kernel_mapped_memory:
    .long 0
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
