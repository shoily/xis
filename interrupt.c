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

#define ASSERT(x)

#define NUM_OF_INTERRUPTS 24

struct ioapic ioapics[1];
struct interrupt *interrupts[MAX_NUM_SMPS];
struct interrupt interrupts_cpu_0[NUM_OF_INTERRUPTS];

struct bus buses[2];

bool ioapic_initialized;

void ioapic_init() {	
	ioapic_initialized = false;
}

void interrupts_init() {

    int i;

    buses[0].id = 0;
    buses[0].type = BUS_TYPE_ISA;

    interrupts[0] = interrupts_cpu_0;

    for(i = 0; i < 16; i++) {

        interrupts_cpu_0[i].orig_irq = i;
        interrupts_cpu_0[i].bus = 0;
        interrupts_cpu_0[i].gsi = 0;
        SET_IRQ_POLARITY(interrupts_cpu_0[i].flags, DEFAULT_ISA_IRQ_POLARITY);
        SET_IRQ_TRIGGER_MODE(interrupts_cpu_0[i].flags, DEFAULT_ISA_IRQ_TRIGGER_MODE);
    }
}

void interrupt_set_override(u8 bus, u8 source, u32 gsi, u16 flags) {

    if (source != gsi) {
        SET_IRQ_FLAGS(interrupts_cpu_0[source].flags, IRQ_FLAG_OVERRIDEN);
        interrupts_cpu_0[gsi].orig_irq = source;
    }

    interrupts_cpu_0[gsi].bus = bus;

    if (GET_IRQ_POLARITY(flags) == 0)
        SET_IRQ_POLARITY(interrupts_cpu_0[gsi].flags, DEFAULT_ISA_IRQ_POLARITY);
    else
        SET_IRQ_POLARITY(interrupts_cpu_0[gsi].flags, GET_IRQ_POLARITY(flags));

    if (GET_IRQ_TRIGGER_MODE(flags) == 0)
        SET_IRQ_TRIGGER_MODE(interrupts_cpu_0[gsi].flags, DEFAULT_ISA_IRQ_TRIGGER_MODE);
    else
        SET_IRQ_TRIGGER_MODE(interrupts_cpu_0[gsi].flags, GET_IRQ_TRIGGER_MODE(flags));
}

void ioapic_write(u8 id, u8 reg, void *data, u8 size) {
    *(u8 *)ioapics[id].base_register = reg;
    *((u32 *)ioapics[id].base_register+0x10) = *(u32 *)data;
    if (size == sizeof(long long)) {
        *((u32 *)ioapics[id].base_register+0x11) = *(u32 *)(data+1);
    }
}

void ioapic_read(u8 id, u8 reg, u8 size, void *data) {
    *(u8 *)ioapics[id].base_register = reg;
    *(u32 *) data = *((u32 *)ioapics[id].base_register+0x10);
    if (size == sizeof(long long)) {
         *(u32 *)(data+1) = *((u32 *)ioapics[id].base_register+0x11);
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
    *max_redir_entries = (temp >> 24) & 0xff;
}

struct ioapic *ioapic_allocate(u8 ioapic_id, u32 base_register, u32 gsi_base) {
    //u8 id, version;

    ASSERT(ioapic_id == 0);
    ioapics[ioapic_id].id = ioapic_id;
    ioapics[ioapic_id].base_register = base_register;
    ioapics[ioapic_id].gsi_base = gsi_base;

    //ioapic_read_id(ioapic_id, &id);

    //ioapic_read_version(ioapic_id, &version, &ioapics[ioapic_id].max_redir_entries);

    //printf(KERNEL_INFO, "IOAPIC: id %d, version %d, max_redir_entries: %d\n", id, version, &ioapics[ioapic_id].max_redir_entries);

    return ioapics;
}
