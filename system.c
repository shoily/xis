/*****************************************************************************/
/*  File: system.c                                                           */
/*                                                                           */
/*  Description: Source file for x86 system information.                     */
/*  Contains E820 map and VGA output code.                                   */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Mar 28, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

//
// Reserved memory aside from E820
//
// 

#include "system.h"
#include "util.h"
#include "lock.h"
#include "debug.h"

#define PIT_FREQUENCY 1000

short *vga_buf_ptr;
//int vga_buffer_line = 0;

spinlock spinlock_vga;

int ebda;

void vga_init() {

	INIT_SPIN_LOCK(&spinlock_vga);
	vga_buf_ptr = (short*)VIDEO_VIRT_BUFFER;
}

//
// Outputs a NULL terminated string in VGA buffer
//
void print_vga(char *c) {

	spinlock_lock(&spinlock_vga);

    while(*c) {

		if (*c == '\n') {

			vga_buf_ptr += (160-(((int)vga_buf_ptr-VIDEO_VIRT_BUFFER)%160))/sizeof(short);
			c++;
			continue;
		}

		if (vga_buf_ptr >= (short*)(VIDEO_VIRT_BUFFER+(160*25)))
			break;

		*vga_buf_ptr = ((0xF << 8) | *c);
        c++;
        vga_buf_ptr++;
    }

	spinlock_unlock(&spinlock_vga);
}

void print_vga_fixed(char *c, int col, int row) {

    unsigned short *p = (unsigned short *)((char*)VIDEO_VIRT_BUFFER+col+(row*160));

    while(*c) {

        if (row > 25) break;
    
        *p = ((0xF << 8) | *c);
        c++;
        col += 2;
        p++;
        if (col == 160) {
            row++;
            col = 0;
        }
    }
}

//
// Dumps E820 map
//
void dump_e820() {

    int i;
    int e820_count= *((int *) E820_MAP_VIRT_COUNT);
    struct e820_map *e820 = (struct e820_map *) E820_MAP_VIRT_ADDRESS;

    printf(KERNEL_DEBUG, "E820 map\n");
    printf(KERNEL_DEBUG, "===========================================================\n");

    printf(KERNEL_DEBUG, "Base address | Length | Type\n");

    for(i=0;i<e820_count;i++, e820++) {
        printf(KERNEL_DEBUG, "0x%p | 0x%p | %d %s\n", (long)e820->base, (long)e820->length, e820->type, (e820->type == 1) ? " Free memory" : " Reserved memory");
    }
}

void mask_pic_8259() {

    __asm__ __volatile__("movb $0xff, %%al;"  // mask master and slave PICs
                         "outb %%al, $0x21;"
                         "outb %%al, $0xa1;"
                         :
                         :
                         : );
}

void init_pic_8259() {

    __asm__ __volatile__("movb $0xff, %%al;"  // mask master and slave PICs
                         "outb %%al, $0x21;"
                         "outb %%al, $0xa1;"
                         "movb $0x11, %%al;"  // init PIC1
                         "outb %%al, $0x20;"
                         "movb $0x20, %%al;"  // map vector 0x20-0x27 for IRQ 0-7
                         "outb %%al, $0x21;"
                         "movb $0x4, %%al;"   // cascade slave at IRQ 2
                         "outb %%al, $0x21;"
                         "movb $0x1, %%al;"   // Regular EOI for PIC1
                         "outb %%al, $0x21;"
                         "movb $0x11, %%al;"  // init PIC2
                         "outb %%al, $0xa0;"
                         "movb $0x28, %%al;"  // map vector 0x28-0x2f for IRQ 8-15
                         "outb %%al, $0xa1;"
                         "movb $0x2, %%al;"   // inform PIC2 that IRQ2 is the connected at PIC1
                         "outb %%al, $0xa1;"
                         "movb $0x1, %%al;"   // manual EOI for PIC2
                         "outb %%al, $0xa1;"
                         "xorl %%ecx, %%ecx;"
                         "movl $0xf000000, %%ecx;"  // small delay
                         "1:;"
                         "loop 1b;"
                         "xorb %%al, %%al;"    // unmask PIC1 and PIC2
                         "outb %%al, $0x21;"
                         "outb %%al, $0xa1;"
                         :
                         :
                         : "%eax", "%ecx"
                         );
}

void init_pit_frequency() {

    __asm__ __volatile__("movb $0x34, %%al;"  // PIT channel 0 and rate generator
                         "outb %%al, $0x43;"  // writing to command register
                         "movw %0, %%ax;"
                         "outb %%al, $0x40;"  // writing low byte of frequency divisor 
                         "rolw $8, %%ax;"
                         "outb %%al, $0x40;"  // then high byte
                         :
                         : "i" (PIT_FREQUENCY)
                         : "%eax"
                         );
}

extern int sched_tick;


void pit_wait(int cycles) {

    int status_reg = 0;
    int counter = 0;

    // uses PIT channel 2 and mode 0 to wait for cycles.

    __asm__ __volatile__("cli;"
                         "movb $0xb0, %%al;"   // PIT channel 2 and mode 0
                         "outb %%al, $0x43;"   // writing to command register
                         "movw %w0, %%ax;"
                         "outb %%al, $0x42;"   // low byte to channel 2
                         "rolw $8, %%ax;"
                         "outb %%al, $0x42;"   // then high byte
                         "1:"
                         "cli;"
                         "movb $0xe8, %%al;"   // read back for status
                         "outb %%al, $0x43;"   // writing to command register
                         "inb $0x42, %%al;"    // read status from channel 2
                         "movb %%al, %b1;"
                         "andb $0x80, %%al;"   // check if OUT is high
                         "sti;"
                         "jnz 1f;"
                         "nop;nop;nop;nop;"    // give chance to PIC to handle interrupts
                         "incl %2;"            // loop count - for debugging purpose
                         "jmp 1b;"
                         "1:"
                         : "=r"(counter), "=r"(status_reg)
                         : "0"(cycles)
                         : "%eax"
                         );

#ifdef DEBUG
    CLI;
    printf(KERNEL_DEBUG, "status_reg: %x", status_reg);
    printf(KERNEL_DEBUG, "counter: %x\n", counter);
    STI;
#endif
}

void pit_wait_ms(int ms) {

    for(int i = 0; i < ms; i++) {

        pit_wait(PIT_HZ);
    }
}

void bda_read_table() {

	short *p = (short*)(0x40e + KERNEL_VIRT_ADDR);

	ebda = ((int)(unsigned short)*p) << 4;

	printf(KERNEL_INFO, "EBDA: %x\n", ebda);
}
