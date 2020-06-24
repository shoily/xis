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
    
        *p = ((0xF << 8) | *c);
        c++;
        vga_buffer_index += 2;
        p++;
        if (vga_buffer_index > 160) {
            vga_buffer_line++;
            vga_buffer_index = 0;
        }
    }

    if (newline) {
        vga_buffer_index = 0;
        vga_buffer_line++;
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
                         "movw $0xffff, %%cx;"  // small delay
                         "1:;"
                         "loop 1b;"
                         "xorb %%al, %%al;"    // unmask PIC1 and PIC2
                         "outb %%al, $0x21;"
                         "outb %%al, $0xa1;"
                         :
                         :
                         : "%eax"
                         );
}

void init_pit_frequency() {

    __asm__ __volatile__("movb $0x34, %%al;"
                         "outb %%al, $0x43;"
                         "movw %0, %%ax;"
                         "outb %%al, $0x40;"
                         "shrw $8, %%ax;"
                         "outb %%al, $0x40;"
                         :
                         : "i" (PIT_FREQUENCY)
                         : "%eax"
                         );
}
