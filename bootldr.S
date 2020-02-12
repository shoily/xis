/*****************************************************************************/
/*  File: bootldr.S                                                          */
/*                                                                           */
/*  Description: This is the boot loader. It should be written at sector 0.  */
/*  It loads the kernel boot code from sector 1 at addres 0x7000,            */
/*  copies to 0x100000 (1MB) address, turns on protected mode and jumps      */
/*  to that address. It uses bootlds.ld to map itself at address  0x7C00     */
/*                                                                           */
/*  Author: Shoily O Rahman                                                  */
/*                                                                           */
/*  Date: Feb 11, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

    // machine boots in 16bit real mode
    .code16
    
    .text
    .globl _start
_start:
    
    // enable A20 line    
    cli
    push %ax
    movb $0xdd, %al
    outb %al, $0x64
    pop %ax
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

    // read only one sector to load kernel
	movw $1, %ax
	movw %ax, 2(%si)

	// size of the disk packet
	movw $0x0010, (%si)

	// kernel is located in sector number 1. Read it.
	movl $1, %ebx
	movl %ebx, 8(%si)
	movl $0, %ebx
	movl %ebx, 12(%si)

	// segment of buffer address
	movw $0x7000, 6(%si)

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
    movb %dl, boot_drive    
    int $0x13

    // loads gdt
    xorw %ax, %ax
    movw %ax, %ds
    lgdt gdtdescriptor

    // turn on protected mode
    movl %cr0, %eax
    orl $1, %eax
    movl %eax, %cr0

    // jump to protected mode
    ljmpl $0x8,$protectedmodeaddress

lba_read_error:
    
    // write 'X' in the screen to indicate read error
    movb $'X', %al
    movb $0x0e, %ah
    int $0x10
    jmp loop

    // protected mode code
    .code32
    
protectedmodeaddress:

    cli

    // data descriptor is located in 0x10 location of GDT
    movl $0x10, %eax
    movl %eax, %ds
    movl %eax, %es
    movl %eax, %ss
    movl %eax, %fs
    movl %eax, %gs

    // initialize protected mode stack
    movl protectedmodestack, %eax
    movl %eax, %esp
    movl %eax, %ebp

    // copy kernel to 1 MB offset
    cld
    movl $0x100, %ecx
    movl $0x70000, %esi
    movl $0x100000, %edi
    rep movsw

    // call kernel at 1 MB
    movl $0x100000, %eax
    call *%eax

loop:
    jmp loop

protectedmodestack:
    .long 0xf000

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
