/*****************************************************************************/
/*  File: interrupt.h                                                        */
/*                                                                           */
/*  Description: Header file for handling interrupt.                         */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Oct 14, 2021                                                       */
/*                                                                           */
/*****************************************************************************/

#ifndef _INTERRUPT_H
#define _INTERRUPT_H

struct int_src_override {
	u8 bus;
	u8 source;
	u32 gsi;
	u16 flags;
};

struct ioapic {
	u8 id;
	u32 base_register;
    u32 gsi_base;
    u8 max_redir_entries;
};

struct interrupt {
    u8 orig_irq;
	u8 bus;
	u32 gsi;
	u16 flags;
};

struct bus {
    u8 id;
    u8 type;
};

#define IRQ_POL_ACTIVE_HIGH 1
#define IRQ_POL_ACTIVE_LOW  3

#define IRQ_TRIGGER_MODE_LEVEL 1
#define IRQ_TRIGGER_MODE_EDGE  3

#define DEFAULT_ISA_IRQ_POLARITY IRQ_POL_ACTIVE_HIGH
#define DEFAULT_ISA_IRQ_TRIGGER_MODE IRQ_TRIGGER_MODE_EDGE

#define IRQ_POL_MASK 0xf
#define IRQ_TRIGGER_MODE_MASK 0xf0
#define IRQ_TRIGGER_MODE_SHIFT 8

#define SET_IRQ_POLARITY(flags, x) (flags |= (x & IRQ_POL_MASK))
#define SET_IRQ_TRIGGER_MODE(flags, x) (flags |= ((x << IRQ_TRIGGER_MODE_SHIFT) & IRQ_TRIGGER_MODE_MASK))

#define GET_IRQ_POLARITY(flags) (flags & IRQ_POL_MASK)
#define GET_IRQ_TRIGGER_MODE(flags) ((flags & IRQ_TRIGGER_MODE_MASK) >> IRQ_TRIGGER_MODE_SHIFT)

#define BUS_TYPE_ISA 0
#define BUS_TYPE_PCI 1

#define IRQ_FLAG_OVERRIDEN 1

#define IRQ_FLAG_MASK 0xf
#define IRQ_FLAG_SHIFT 12

#define SET_IRQ_FLAGS(flags, value) (flags |= ((value << IRQ_FLAG_SHIFT) & IRQ_FLAG_MASK))
#define GET_IRQ_FLAGS(flags) ((flags & IRQ_FLAGS_MASK) >> IRQ_FLAG_SHIFT)

void interrupts_init();
struct ioapic *ioapic_allocate(u8 ioapic_id, u32 base_register, u32 gsi_base);
void interrupt_set_override(u8 bus, u8 source, u32 gsi, u16 flags);
void ioapic_init();

#endif
