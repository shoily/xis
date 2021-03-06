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
#include "krnlconst.h"
#include "apic.h"

#define MAX_NUM_SMPS 32

#define VIDEO_VIRT_BUFFER (KERNEL_VIRT_ADDR+VIDEO_BUFFER)
#define E820_MAP_VIRT_ADDRESS (KERNEL_VIRT_ADDR+E820_MAP_ADDRESS)
#define E820_MAP_VIRT_COUNT (KERNEL_VIRT_ADDR+E820_MAP_COUNT)

#define PIT_CRYSTAL_FREQUENCY 1193182
#define PIT_HZ (PIT_CRYSTAL_FREQUENCY/1000)

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

extern int lapic_present;

#define CUR_CPU (lapic_present ? (lapic_read_register(LAPIC_ID_REG) >> 24) : 0)

struct e820_map {
    unsigned long long base;
    unsigned long long length;
    unsigned int type;
    unsigned int ACPI;
}__attribute__((packed));

void dump_e820();
void mask_pic_8259();
void init_pic_8259();
void init_pit_frequency();
void pit_wait(int cycles);
void pit_wait_ms(int ms);
void vga_init();
void print_vga(char *c);
void print_vga_fixed(char *c, int col, int row);
void bda_read_table();

#endif
