/*****************************************************************************/
/*  File: x86.h                                                              */
/*                                                                           */
/*  Description: Header file for x86 common data structure for 32 and 64 bit.*/
/*  Contains E820 map and VGA buffer address.                                */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Mar 28, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#ifndef _X86_H
#define _x86_H

#include "type.h"

#define VIDEO_BUFFER 0x800b8000
#define E820_MAP_ADDRESS 0x80001008
#define E820_MAP_COUNT 0x80001004

struct e820_map {
    unsigned long long base;
    unsigned long long length;
    unsigned int type;
    unsigned int ACPI;
}__attribute__((packed));

void dump_e820();
void print_vga(char *c, bool newline);

#endif
