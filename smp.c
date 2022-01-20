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

extern int _kernel_pg_dir;
extern int _kernel_pg_table_0;

extern int _kernel_stack_0_start;

int smp_bits = 0;
int smp_nums = 0;

spinlock spinlock_smp;

void finish_smp_initialization(int smp_id) {

    spinlock_lock(&spinlock_smp);

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

	// clear identity mapping
    GET_CURCPU_PGDIR[0] = 0;
    //((int*)((int)&_kernel_pg_dir+(CUR_CPU*PAGE_SIZE)))[0] = 0;
    __asm__ __volatile__("invlpg (0);"
                         : : :
                         );

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
		memcpy(pgd, &_kernel_pg_dir, PAGE_SIZE);
		// identity mapping for first 4 MB
		*pgd = *(pgd+(KERNEL_PGDIR_ENTRY/4));
	}
}

void smp_start() {

    INIT_SPIN_LOCK(&spinlock_smp);

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

    printf(KERNEL_INFO, "Number of APs: %d\n", smp_nums);
    printf(KERNEL_INFO, "SMP bits: %x\n", smp_bits);
}
