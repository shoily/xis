/*****************************************************************************/
/*  File: smp.c                                                              */
/*                                                                           */
/*  Description: Source file SMP related code.                               */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Aug 9, 2020                                                        */
/*                                                                           */
/*****************************************************************************/

#include "util.h"
#include "system.h"
#include "apic.h"

#define AP_INIT_PHYS_TEXT 0xf000
#define AP_COUNT_PHYS_ADDR 0xfff8

extern int mp_init_size;
extern int init_ap;

void copy_smp_init_to_low_mem() {

    char *s = (char*)&init_ap;
    char *d = (char*)(AP_INIT_PHYS_TEXT+KERNEL_VIRT_ADDR);

    for(int i=0;i<(int)&mp_init_size;i++) {
        *d++ = *s++;
    }
}

void smp_start() {

    int status;

    copy_smp_init_to_low_mem();

    // initialize AP processor count with 0
    // it will be increased by AP startup code
    *(int*)(AP_COUNT_PHYS_ADDR+KERNEL_VIRT_ADDR) = 0;

    MFENCE;

    // send INIT IPI to APs
    write_lapic_register(LAPIC_ICR_1, 0x000c4500);
    write_lapic_register(LAPIC_ICR_2, 0);
    for(int i=0;i<10;i++) {
        status = read_lapic_register(LAPIC_ICR_1) & 0x1000;
        if (!status)
            break;
        pit_wait(0xffff);
    }

    // send Startup IPI to APs
    write_lapic_register(LAPIC_ICR_1, 0x000c4600 | (AP_INIT_PHYS_TEXT >> 12));
    write_lapic_register(LAPIC_ICR_2, 0);
    for(int i=0;i<30;i++) {
        status = read_lapic_register(LAPIC_ICR_1) & 0x1000;
        if (!status)
            break;
        pit_wait(0xffff);
    }

    pit_wait(0xffff);

    print_msg("Number of APs", *(int*)(AP_COUNT_PHYS_ADDR+KERNEL_VIRT_ADDR), 10, true);
}
