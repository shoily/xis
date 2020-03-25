/*****************************************************************************/
/*  File: x86_32.h                                                           */
/*                                                                           */
/*  Description: Header file for x86 E820 map                                */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Mar 15, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#ifndef _X86_32_H
#define _X86_32_H

#define VIDEO_BUFFER 0x800b8000
#define E820_MAP_ADDRESS 0x80001008
#define E820_MAP_COUNT 0x80001004

struct e820_map {
    unsigned long long base;
    unsigned long long length;
    unsigned int type;
    unsigned int ACPI;
}__attribute__((packed));

#endif
