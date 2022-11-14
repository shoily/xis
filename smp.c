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
#include "setup32.h"
#include "usermode.h"
#include "page32.h"
#include "debug.h"
#include "smp.h"

extern int init_ap;
extern int init_ap_size;

extern int lapic_base_register;
extern int _kernel_pg_table_0;
extern int _kernel_stack_0_start;
extern int _master_kernel_pg_dir;

int smp_bits = 1;
int smp_nums = 0;
int smp_on_hold = 0;

spinlock spinlock_smp;

void finish_smp_initialization(int smp_id) {
retry:
    spinlock_lock(&spinlock_smp);
    if (smp_on_hold) {
        spinlock_unlock(&spinlock_smp);
        goto retry;
    }

    smp_nums++;
    smp_bits |= 1 << smp_id;

    spinlock_unlock(&spinlock_smp);

    loadGDT32();
    loadLDT32();
    initializeTSS32(smp_id);
    loadTSS32(smp_id);
    loadIDT32();

    lapic_write_register(LAPIC_SPURIOUS_REG, lapic_read_register(LAPIC_SPURIOUS_REG)| 0x1ff);
    lapic_write_register(LAPIC_LVT_TIMER_REG, LAPIC_IDT_VECTOR | 0x20000); // Periodic timer on vector 32.
    lapic_write_register(LAPIC_DIVIDE_CONFIGURATION_REG, LAPIC_DIVIDE_CONFIG_VALUE); // Divide by 128
    lapic_write_register(LAPIC_INITIAL_COUNTER_REG, LAPIC_COUNTER_VALUE);

    STI;
    initialize_usermode();
    switch_to_um();
}

void copy_smp_init_to_low_mem() {

    char *s = (char*)&init_ap;
    char *d = (char*)(AP_INIT_PHYS_TEXT+KERNEL_VIRT_ADDR);

    for(int i=0;i<(int)init_ap_size;i++) {
        *d++ = *s++;
    }
}

void initialize_kernel_pg_tables() {

    pgd_t *pgd;

    for(int i = 1; i < MAX_NUM_SMPS; i++) {

        pgd = (pgd_t*)((int)&_kernel_pg_dir + (PAGE_SIZE * i));
        memcpy(pgd, &_master_kernel_pg_dir, PAGE_SIZE);
        // identity mapping for first 4 MB
        *pgd = *(pgd+KERNEL_PGDIR_ENTRY);
    }
}

void smp_start() {

    if (!lapic_present)
        return;

    copy_smp_init_to_low_mem();

    initialize_kernel_pg_tables();

    // initialize AP processor count with 0
    // it will be increased by AP startup code
    *(int*)(AP_FIRST_STACK+KERNEL_VIRT_ADDR) = (int)&_kernel_stack_0_start;
    *(int*)(AP_LAPIC_BASE_REGISTER+KERNEL_VIRT_ADDR) = (int)lapic_base_register;
    *(int*)(AP_KERNEL_PG_DIR+KERNEL_VIRT_ADDR) = ((int)&_kernel_pg_dir-KERNEL_VIRT_ADDR);
    *(int*)(AP_FINISH_CODE+KERNEL_VIRT_ADDR) = (int)finish_smp_initialization;

    MFENCE;

    // send INIT IPI to APs
    lapic_write_register(LAPIC_ICR_1, 0x000c4500);
    lapic_write_register(LAPIC_ICR_2, 0);
    pit_wait_ms(10);

    // send Startup IPI to APs
    lapic_write_register(LAPIC_ICR_1, 0x000c4600 | (AP_INIT_PHYS_TEXT >> 12));
    lapic_write_register(LAPIC_ICR_2, 0);
    pit_wait_ms(200);

    printf(KERNEL_INFO, "Number of APs: %d ", smp_nums);
    printf(KERNEL_INFO, "SMP bits: %x\n", smp_bits);
}
