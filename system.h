/*****************************************************************************/
/*  File: system.h                                                           */
/*                                                                           */
/*  Description: Header file for x86 system information.                     */
/*  Contains E820 map and VGA buffer address.                                */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Mar 28, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#ifndef _SYSTEM_H
#define _SYSTEM_H

#include "type.h"

#define KERNEL_VIRT_ADDR 0x80000000
#define VIDEO_BUFFER (KERNEL_VIRT_ADDR+0xb8000)
#define E820_MAP_ADDRESS (KERNEL_VIRT_ADDR+0x1008)
#define E820_MAP_COUNT (KERNEL_VIRT_ADDR+0x1004)

#define MFENCE __asm__ __volatile__("mfence;"    \
                                    :            \
                                    :            \
                                    : "memory"   \
                                    );           \


#define CLI __asm__ __volatile__("cli;"       \
                                 :            \
                                 :            \
                                 :            \
                                 );           \

#define STI __asm__ __volatile__("sti;"       \
                                 :            \
                                 :            \
                                 :            \
                                 );           \

struct e820_map {
    unsigned long long base;
    unsigned long long length;
    unsigned int type;
    unsigned int ACPI;
}__attribute__((packed));

void dump_e820();
void print_vga(char *c, bool newline);
void init_pic_8259();
void init_pit_frequency();
void pit_wait(int cycles);

#endif
