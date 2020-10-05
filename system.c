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

#define PIT_FREQUENCY 1000

int vga_buffer_index = 0;
int vga_buffer_line = 0;

//
// Outputs a NULL terminated string in VGA buffer
//
void print_vga(char *c, bool newline) {

    unsigned short *p = (unsigned short *)((char*)VIDEO_BUFFER+vga_buffer_index+(vga_buffer_line*160));

    while(*c) {

        if (vga_buffer_line > 25) break;
    
        *p = ((0xF << 8) | *c);
        c++;
        vga_buffer_index += 2;
        p++;
        if (vga_buffer_index == 160) {
            vga_buffer_line++;
            vga_buffer_index = 0;
        }
    }

    if (newline) {
        vga_buffer_index = 0;
        vga_buffer_line++;
    }
}

void print_vga_fixed(char *c, int col, int row) {

    unsigned short *p = (unsigned short *)((char*)VIDEO_BUFFER+col+(row*160));

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
    int e820_count= *((int *) E820_MAP_COUNT);
    struct e820_map *e820 = (struct e820_map *) E820_MAP_ADDRESS;
    char str[17];

    print_vga("E820 map", true);
    print_vga("===========================================================", true);

    print_vga("Base address       | Length             | Type", true);

    for(i=0;i<e820_count;i++, e820++) {
        print_vga("0x", false);
        lltoa(e820->base, str, 16);
        print_vga(str, false);
        print_vga(" | ", false);
        print_vga("0x", false);
        lltoa(e820->length, str, 16);
        print_vga(str, false);
        print_vga(" | ", false);
        itoa(e820->type, str, 16);
        print_vga(str, false);
        print_vga((e820->type == 1) ? " Free memory" : " Reserved memory", true);
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
    print_msg("status_reg", status_reg, 16, false);
    print_msg("counter", counter, 10, true);
    STI;
#endif
}

void pit_wait_ms(int ms) {

    for(int i = 0; i < ms; i++) {

        pit_wait(PIT_HZ);
    }
}
