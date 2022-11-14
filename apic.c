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

int lapic_present = 0;
int lapic_base_register;
int lapic_id;

extern addr_t _kernel_pg_dir;

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
    int local_lapic_present = 0;

    __asm__ __volatile__("movl $1, %%eax;"
                         "cpuid;"
                         "andl $0x200, %%edx;"
                         "shrl $9, %%edx;"
                         "movl %%edx, %0;"
                         : "=r" (local_lapic_present)
                         :
                         : "%eax", "%edx", "memory"
                         );

    printf(KERNEL_INFO, "Local APIC present: %d ", local_lapic_present);

    if (!local_lapic_present) {
        return;
    }

    read_msr(0x1b, &eax, &edx);
    lapic_base_register = eax & 0xfffff000;

    map_kernel_with_pagetable(lapic_base_register, lapic_base_register, PAGE_SIZE, PTE_WRITE, 0);

    lapic_present = local_lapic_present;

    printf(KERNEL_INFO, "Local APIC address: %p ", eax);

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
