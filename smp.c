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
#include "krnlconst.h"
#include "lock.h"

#define AP_INIT_PHYS_TEXT 0x7c00

extern int init_ap;
extern int init_ap_size;

extern int lapic_base_register;

extern int _kernel_pg_dir;
extern int _kernel_pg_table_0;

extern int _kernel_stack_0_start;

int smp_bits = 0;

spin_lock spinlock_smp;

void finish_smp_initialization(int lapic_id) {

    spinlock_lock(&spinlock_smp);

    smp_bits |= 1 << lapic_id;
    print_msg("Init SMP lapic id", lapic_id, 16, false);

    spinlock_unlock(&spinlock_smp);
}

void copy_smp_init_to_low_mem() {

    char *s = (char*)&init_ap;
    char *d = (char*)(AP_INIT_PHYS_TEXT+KERNEL_VIRT_ADDR);

    print_msg("init_ap_size", (int)init_ap_size, 10, true);

    for(int i=0;i<(int)init_ap_size;i++) {
        *d++ = *s++;
    }
}

void smp_start() {

    INIT_SPIN_LOCK(&spinlock_smp);

    copy_smp_init_to_low_mem();

    // initialize AP processor count with 0
    // it will be increased by AP startup code
    *(int*)(AP_COUNT_PHYS_ADDR+KERNEL_VIRT_ADDR) = 0;
    *(int*)(AP_FIRST_STACK+KERNEL_VIRT_ADDR) = (int)&_kernel_stack_0_start;
    *(int*)(AP_LAPIC_BASE_REGISTER+KERNEL_VIRT_ADDR) = (int)lapic_base_register;
    *(int*)(AP_KERNEL_PG_DIR+KERNEL_VIRT_ADDR) = (((int)&_kernel_pg_dir)-KERNEL_VIRT_ADDR);
    *(int*)(AP_FINISH_CODE+KERNEL_VIRT_ADDR) = (int)finish_smp_initialization;

    // identity mapping to enable APs to use paging.
    // It needs to be cleared after APs are initialized or it usermode programs
    // sharing this page directory cannot access first 4MB virtual memory.
    ((int*)&_kernel_pg_dir)[0] = (((int)&_kernel_pg_table_0) - KERNEL_VIRT_ADDR) | 1;

    MFENCE;

    // send INIT IPI to APs
    write_lapic_register(LAPIC_ICR_1, 0x000c4500);
    write_lapic_register(LAPIC_ICR_2, 0);
    pit_wait_ms(10);

    // send Startup IPI to APs
    write_lapic_register(LAPIC_ICR_1, 0x000c4600 | (AP_INIT_PHYS_TEXT >> 12));
    write_lapic_register(LAPIC_ICR_2, 0);
    pit_wait_ms(200);

    // clear identity mapping
    ((int*)&_kernel_pg_dir)[0] = 0;
    __asm__ __volatile__("invlpg (0);"
                         : : :
                         );

    print_msg("Number of APs", *(int*)(AP_COUNT_PHYS_ADDR+KERNEL_VIRT_ADDR), 10, true);
    print_msg("SMP bits", smp_bits, 16, true);
}
