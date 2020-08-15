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

int lapic_present = 0;
int lapic_base_register;
int lapic_id;

#define ADD2PTR(x, y) ((int)(x) + (int)(y))

extern int _kernel_pg_dir;

char __attribute__((aligned(4096))) lapic_pg_table[4096];

void read_msr(int msr, int *eax, int *edx) {

    __asm__ __volatile__("rdmsr;"
                         : "=a" (*eax), "=d" (*edx)
                         : "c" (msr)
                         : );
}

int read_lapic_register(int lapic_register) {
    
    return *((int*)(lapic_base_register+lapic_register));
}

void write_lapic_register(int lapic_register, int value) {
    
    *((int*)(lapic_base_register+lapic_register)) = value;
}

void init_lapic() {

    int eax, edx;  
    pte_t *pgtable[1];

    __asm__ __volatile__("movl $1, %%eax;"
                         "cpuid;"
                         "andl $0x200, %%edx;"
                         "shrl $9, %%edx;"
                         "movl %%edx, %0;"
                         : "=r" (lapic_present)
                         :
                         : "%eax", "%edx"
                         );

    print_msg("Local APIC present", lapic_present, 10, true);

    if (!lapic_present) {

        return;
    }

    memset(lapic_pg_table, sizeof(lapic_pg_table), 0);
    pgtable[0] = (pte_t*)lapic_pg_table;

    read_msr(0x1b, &eax, &edx);
    lapic_base_register = eax & 0xfffff000;

    build_pagetable((pgd_t*)&_kernel_pg_dir, pgtable, lapic_base_register, lapic_base_register, PAGE_SIZE, PGD_PRESENT | PGD_WRITE, PTE_PRESENT | PTE_WRITE);
    
    print_msg("Local APIC address", eax, 16, false);

    lapic_id = read_lapic_register(LAPIC_ID_REG) >> 24;

    print_msg("Local APIC id", lapic_id, 16, true);

    // enable receiving interrupt
    write_lapic_register(LAPIC_SPURIOUS_REG, read_lapic_register(LAPIC_SPURIOUS_REG)| 0x100);
}

void lapic_switch(bool enable) {
    
    int value;
    
    value = read_lapic_register(LAPIC_SPURIOUS_REG);
    if (enable)
        value |= 0x1ff;
    else
        value &= ~0x1ff;

    print_msg("lapic_switch", value, 16, true);
    
    write_lapic_register(LAPIC_SPURIOUS_REG, value);
}
