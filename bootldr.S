/*****************************************************************************/
/*  File: bootldr.S                                                          */
/*                                                                           */
/*  Description: This is the boot loader. It should be written at sector 0.  */
/*  It uses bootlds.ld to map itself at address  0x7C00                      */
/*  1. Enumerates and stores E820 map                                        */
/*  2. Loads the kernel boot code from sector 1 at addres 0x5000             */
/*  3. Copies to 0x100000 (1MB) address, turns on protected mode and jumps   */
/*  to that address.                                                         */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Feb 11, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

    .include "krnlsize.h"
    .equ E820_MAP_ADDR, 0x1008
    .equ PROTECTED_MODE_STACK, 0xf000
    .equ KERNEL_SECTORS, ((KERNEL_SIZE + 0x200 -1) / 0x200)
    .equ PIT_FREQUENCY, 1000

    // machine boots in 16bit real mode
    .code16
    
    .text
    .globl _start
_start:
    movb %dl, boot_drive

    // initialize PIC

    // mask master and slave PICs
    movb $0xff, %al
    outb %al, $0x21
    outb %al, $0xa1

    // init PIC1
    movb $0x11, %al
    outb %al, $0x20

    // map vector 0x20-0x27 for IRQ 0-7
    movb $0x20, %al
    outb %al, $0x21

    // cascade slave at IRQ 2
    movb $0x4, %al
    outb %al, $0x21

    // Regular EOI for PIC1
    movb $0x1, %al
    outb %al, $0x21

    // init PIC2
    movb $0x11, %al
    outb %al, $0xa0

    // map vector 0x28-0x2f for IRQ 8-15
    movb $0x28, %al
    outb %al, $0xa1

    // inform PIC2 that IRQ2 is the connected at PIC1
    movb $0x2, %al
    outb %al, $0xa1

    // manual EOI for PIC2
    movb $0x1, %al
    outb %al, $0xa1

    // small delay
    movw $0xffff, %cx
1:
    loop 1b

    // unmask PIC1 and PIC2
    xorb %al, %al
    outb %al, $0x21
    outb %al, $0xa1

    // set PIT frequency
    cli
    movb $0x34, %al
    outb %al, $0x43
    movw PIT_FREQUENCY, %ax
    outb %al, $0x40
    shrw $8, %ax
    outb %al, $0x40
    sti

    // clear screen
    movb $0, %ah
    movb $3, %al
    int $0x10

    // reading e820 map
    xorl %eax, %eax
    movl %eax, %ds
    movl %eax, %es
    movl %eax, %ss

    movl %eax, %ebp

    // initializing e820 call
    movl $0xe820, %eax
    xorl %ebx, %ebx
    movl $24, %ecx
    movl $0x534d4150, %edx
    movl $E820_MAP_ADDR, %edi
    movl $1, 20(%edi)
    int $0x15

    jc e820_failed
    movl $0x534d4150, %edx
    cmpl %eax, %edx
    jne e820_failed
    testl %ebx, %ebx
    je e820_failed

e820_next_entry:

    //looping through e820 next entries
    movl $24, %ecx
    addl %ecx, %edi
    movl $1, 20(%edi)

    movl $0xe820, %eax
    movl $0x534d4150, %edx
    int $0x15

    jc e820_done
    jcxz e820_done
    testl %ebx, %ebx
    je e820_done
    cmpl $20, %ecx
    jne e820_failed

    incl %ebp

    jmp e820_next_entry

e820_failed:

    // write 'E' in the screen to indicate e820 error
    movb $'E', %al
    movb $0x0e, %ah
    int $0x10

    jmp a20_line

e820_done:

    // store e820 count in memory
    movl %ebp, E820_MAP_ADDR-4

    // write '8' in the screen to indicate e820 success
    movb $'8', %al
    movb $0x0e, %ah
    int $0x10

a20_line:

    xorl %edx, %edx
    movb boot_drive, %dl
    
    // enable A20 line    
    cli
    movb $0xdd, %al
    outb %al, $0x64
    sti

    // read kernel from second sector    
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss

    // memory buffer location of disk address packet
    movw dap, %si
    xorw %ax, %ax
	movw %ax, 4(%si)

    movw $KERNEL_SECTORS, %ax
    
    // check kernel can fit into existing memory
    cmpw $160, %ax
    ja kernel_too_big

    // read enough sectors to load kernel
    movw %ax, 2(%si)

    // size of the disk packet
    movw $0x0010, (%si)

    // kernel is located in sector number 1. Read it.
    movl $1, %ebx
    movl %ebx, 8(%si)
    movl $0, %ebx
    movl %ebx, 12(%si)

    // segment of buffer address
    movw $0x5000, 6(%si)

    // set stack location at 4K offset
    movl $0x1000, %ebp
    movl %ebp, %esp

    // issue int 13h function 42h to read sector
    movb $0x42, %ah
	int	$0x13

    jc lba_read_error

    // initialize protected mode
    cli
    xor %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    sti

    // resets disk system
    int $0x13

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
    ljmpl $0x8,$protectedmodeaddress

lba_read_error:
    
    // write 'X' in the screen to indicate read error
    movb $'X', %al
    movb $0x0e, %ah
    int $0x10
    jmp loop

kernel_too_big:
    // write 'B' in the screen to indicate kernel too big
    movb $'X', %al
    movb $0x0e, %ah
    int $0x10
    jmp loop

    // protected mode code
    .code32
    
protectedmodeaddress:

    // data descriptor is located in 0x10 location of GDT
    movl $0x10, %eax
    movl %eax, %ds
    movl %eax, %es
    movl %eax, %ss
    movl %eax, %fs
    movl %eax, %gs

    // initialize protected mode stack
    movl PROTECTED_MODE_STACK, %eax
    movl %eax, %esp
    movl %eax, %ebp

    // copy kernel to 1 MB offset
    cld
    movl $(KERNEL_SECTORS * 0x200 / 4), %ecx
    movl $0x50000, %esi
    movl $0x100000, %edi
    rep movsl

    // call kernel at 1 MB
    movl $0x100000, %eax
    call *%eax

loop:
    jmp loop

    .data

boot_drive:
    .byte 0
mode:
	.byte	0
dap:              /* disk address packet */
	.long	0
	.long	0
    .long   0
    .long   0

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

gdtdescriptor:
    .word end_of_gdt-gdt-1
    .long gdt

    .equ code_seg, code_seg_addr-gdt
    .equ data_seg, data_seg_addr-gdt

    // boot loader signature
    .section ".bootsig","a"
    .byte 0x55
    .byte 0xaa
