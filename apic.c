/*****************************************************************************/
/*  File: apic.c                                                             */
/*                                                                           */
/*  Description: Source file for IOAPIC and Local APIC code.                 */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Aug 2, 2020                                                        */
/*                                                                           */
/*****************************************************************************/

#include "apic.h"
#include "page32.h"
#include "util.h"
#include "system.h"
#include "setup32.h"
#include "debug.h"

int lapic_present;
int lapic_base_register;
int lapic_id;

extern addr_t _kernel_pg_dir;

char __attribute__((aligned(4096))) lapic_pg_table[4096];

void read_msr(int msr, int *eax, int *edx) {

    __asm__ __volatile__("rdmsr;"
                         : "=a" (*eax), "=d" (*edx)
                         : "c" (msr)
                         : );
}

int lapic_read_register(int lapic_register) {
    
    return *((int*)(lapic_base_register+lapic_register));
}

void lapic_write_register(int lapic_register, int value) {
    
    *((int*)(lapic_base_register+lapic_register)) = value;
}

extern int lapic_calibration_tick;
extern bool lapic_calibration_mode;
extern bool lapic_timer_enabled;

void calibrate_lapic_timer() {

	lapic_write_register(LAPIC_LVT_TIMER_REG, LAPIC_IDT_VECTOR | 0x20000); // Periodic timer on vector 32.
    lapic_write_register(LAPIC_DIVIDE_CONFIGURATION_REG, LAPIC_DIVIDE_CONFIG_VALUE); // Divide by 128

    lapic_calibration_mode = true;
    lapic_write_register(LAPIC_INITIAL_COUNTER_REG, LAPIC_COUNTER_VALUE);
    pit_wait_ms(1);
	CLI;
    lapic_write_register(LAPIC_INITIAL_COUNTER_REG, 0);
    lapic_calibration_mode = false;
	lapic_timer_enabled = true;
    STI;
	lapic_write_register(LAPIC_INITIAL_COUNTER_REG, LAPIC_COUNTER_VALUE);
}

void init_lapic() {

    int eax, edx;
    pte_t *pgtable[1];

	lapic_present = 0;

    __asm__ __volatile__("movl $1, %%eax;"
                         "cpuid;"
                         "andl $0x200, %%edx;"
                         "shrl $9, %%edx;"
                         "movl %%edx, %0;"
                         : "=r" (lapic_present)
                         :
                         : "%eax", "%edx"
                         );

	printf(KERNEL_INFO, "Local APIC present: %d\n", lapic_present);

    if (!lapic_present) {

        return;
    }

    memset(lapic_pg_table, 0, sizeof(lapic_pg_table));
    pgtable[0] = (pte_t*)lapic_pg_table;

    read_msr(0x1b, &eax, &edx);
    lapic_base_register = eax & 0xfffff000;

    build_pagetable(0, pgtable, lapic_base_register, lapic_base_register, PAGE_SIZE, PGD_PRESENT | PGD_WRITE, PTE_PRESENT | PTE_WRITE);

	printf(KERNEL_INFO, "Local APIC address: %p\n", eax);

    lapic_id = lapic_read_register(LAPIC_ID_REG) >> 24;

    printf(KERNEL_INFO, "Local APIC id: %x\n", lapic_id);

    // enable receiving interrupt
    lapic_write_register(LAPIC_SPURIOUS_REG, lapic_read_register(LAPIC_SPURIOUS_REG)| 0x100);

    calibrate_lapic_timer();
}

void lapic_switch(bool enable) {
    
    int value;
    
    value = lapic_read_register(LAPIC_SPURIOUS_REG);
    if (enable)
        value |= 0x1ff;
    else
        value &= ~0x1ff;

    lapic_write_register(LAPIC_SPURIOUS_REG, value);
}
