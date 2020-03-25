/*****************************************************************************/
/*  File: start.c                                                            */
/*                                                                           */
/*  Description: Kernel initialization code.                                 */
/*  start_kernel routine is called from boot32.S.                            */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Feb 11, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#include "type.h"
#include "common.h"
#include "x86_32.h"


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

//
//  Start kernel routine
//

int start_kernel(void) {

    print_vga("XIS kernel started (v1.0)", true);
    print_vga("", true);
    dump_e820();

    return 0;
}
