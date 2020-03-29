/*****************************************************************************/
/*  File: x86.c                                                              */
/*                                                                           */
/*  Description: Source file for x86 common code for 32 and 64 bit.          */
/*  Contains E820 map and VGA output code.                                   */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Mar 28, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#include "x86.h"
#include "common.h"

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
