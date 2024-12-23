/*****************************************************************************/
/*  File: interrupt.c                                                        */
/*                                                                           */
/*  Description: Source file for handling interrupt.                         */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Oct 14, 2021                                                       */
/*                                                                           */
/*****************************************************************************/

#include "system.h"
#include "interrupt.h"
#include "debug.h"
#include "page32.h"
#include "util.h"
#include "apic.h"

#define ASSERT(x)

#define NUM_OF_INTERRUPTS 24

struct ioapic ioapics[1];
struct interrupt *interrupts[MAX_NUM_SMPS];
struct interrupt interrupts_cpu_0[NUM_OF_INTERRUPTS];

struct bus buses[2];

bool ioapic_initialized;

void interrupts_init() {

    int i;

    buses[0].id = 0;
    buses[0].type = BUS_TYPE_ISA;

    interrupts[0] = interrupts_cpu_0;

    for(i = 0; i < 16; i++) {

        interrupts_cpu_0[i].orig_irq = i;
        interrupts_cpu_0[i].bus = 0;
        interrupts_cpu_0[i].gsi = i;
        interrupts_cpu_0[i].polarity = DEFAULT_ISA_IRQ_POLARITY;
        interrupts_cpu_0[i].trigger = DEFAULT_ISA_IRQ_TRIGGER_MODE;
    }
}

void interrupt_set_override(u8 bus, u8 source, u32 gsi, u8 polarity, u8 trigger) {

    if (source != gsi) {
        interrupts_cpu_0[source].overriden = true;
        interrupts_cpu_0[gsi].orig_irq = source;
    }

    interrupts_cpu_0[gsi].bus = bus;

    if (polarity == 0)
        interrupts_cpu_0[gsi].polarity = DEFAULT_ISA_IRQ_POLARITY;
    else
        interrupts_cpu_0[gsi].polarity = polarity;

    if (trigger == 0)
        interrupts_cpu_0[gsi].trigger = DEFAULT_ISA_IRQ_TRIGGER_MODE;
    else
        interrupts_cpu_0[gsi].trigger = trigger;
}

void interrupts_enable() {
    init_lapic();

    if (!lapic_present) {
        init_pic_8259();
        init_pit_frequency();
    }
    device_enable_interrupts();
}

void ioapic_write(u8 id, u8 reg, void *data, u8 size) {
    *(u8 *)ioapics[id].base_register = reg;
    *(u32 *)(ioapics[id].base_register+0x10) = *(u32 *)data;
    if (size == sizeof(long long)) {
        *(u32 *)(ioapics[id].base_register+0x11) = *(u32 *)(data+1);
    }
}

void ioapic_read(u8 id, u8 reg, u8 size, void *data) {
    *(u8 *)ioapics[id].base_register = reg;
    *(u32 *)data = *(u32 *)(ioapics[id].base_register+0x10);
    if (size == sizeof(long long)) {
        *(u32 *)(data+1) = *(u32 *)(ioapics[id].base_register+0x11);
    }
}

void ioapic_read_id(u8 id, u8 *ioapic_id) {
    u32 temp;
    ioapic_read(id, 0, sizeof(u32), &temp);
    *ioapic_id = (temp >> 24) & 0xf;
}

void ioapic_read_version(u8 id, u8 *version, u8 *max_redir_entries) {
    u32 temp;
    ioapic_read(id, 1, sizeof(u32), &temp);
    *version = temp & 0xff;
    *max_redir_entries = ((temp >> 16) & 0xff)+1;
}

extern int lapic_id;

struct ioapic *ioapic_allocate(u8 ioapic_id, u32 base_register, u32 gsi_base) {
    u8 id, version;

    ASSERT(ioapic_id == 0);
    ioapics[ioapic_id].id = ioapic_id;
    ioapics[ioapic_id].base_register = base_register;
    ioapics[ioapic_id].gsi_base = gsi_base;

    map_kernel_with_pagetable(base_register, base_register, PAGE_SIZE, PTE_WRITE, 0);
    ioapic_read_id(ioapic_id, &id);
    ioapic_read_version(ioapic_id, &version, &ioapics[ioapic_id].max_redir_entries);

    printf(KERNEL_INFO, "IOAPIC: id %d, version %d, max_redir_entries: %d\n", id, version, ioapics[ioapic_id].max_redir_entries);

    return ioapics;
}

void ioapic_setisa_irqs() {
    struct ioapic_entry e;
    int i;

    for(i = 0; i < 16; i++) {
        if (interrupts[0][i].overriden ||
            interrupts[0][i].orig_irq == 0)
            continue;
        memset(&e, 0, sizeof(e));
        e.vector = i + LAPIC_LVT_TIMER_REG + 1;
        e.delivery_mode = IOAPIC_DEL_MODE_FIXED;
        e.destination_mode = IOAPIC_DEST_MODE_PHYSICAL;
        e.polarity = interrupts[0][i].polarity == IRQ_POL_ACTIVE_LOW ? IOAPIC_POLARITY_ACTIVE_LOW : IOAPIC_POLARITY_ACTIVE_HIGH;
        e.trigger = interrupts[0][i].trigger == IRQ_TRIGGER_MODE_LEVEL ? IOAPIC_TRIGGER_LEVEL : IOAPIC_TRIGGER_EDGE;
        e.destination = lapic_id;

        ioapic_write(0, 0x10+(i*2), &e.low, 32);
        ioapic_write(0, 0x11+(i*2), &e.high, 32);
    }
}

void ioapic_init() {
    ioapic_initialized = true;
    ioapic_setisa_irqs();
}
