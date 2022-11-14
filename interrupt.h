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

#define IOAPIC_DEL_MODE_FIXED       0
#define IOAPIC_DEL_MODE_LOWEST_PRIO 1
#define IOAPIC_DEL_MODE_SMI         2
#define IOAPIC_DEL_MODE_NMI         4
#define IOAPIC_DEL_MODE_INIT        5
#define IOAPIC_DEL_MODE_EXT_INT     7

#define IOAPIC_DEST_MODE_PHYSICAL 0
#define IOAPIC_DEST_MODE_LOGICAL 1

#define IOAPIC_DEL_STATUS_RELAXED 0
#define IOAPIC_DEL_STATUS_WAITING 1

#define IOAPIC_POLARITY_ACTIVE_HIGH 0
#define IOAPIC_POLARITY_ACTIVE_LOW 1

#define IOAPIC_TRIGGER_EDGE 0
#define IOAPIC_TRIGGER_LEVEL 1

#define IRQ_POL_ACTIVE_HIGH 1
#define IRQ_POL_ACTIVE_LOW  3

#define IRQ_TRIGGER_MODE_EDGE  1
#define IRQ_TRIGGER_MODE_LEVEL 3

#define DEFAULT_ISA_IRQ_POLARITY IRQ_POL_ACTIVE_HIGH
#define DEFAULT_ISA_IRQ_TRIGGER_MODE IRQ_TRIGGER_MODE_EDGE

#define IRQ_POL_MASK 0x2
#define IRQ_TRIGGER_MODE_MASK 0x2
#define IRQ_TRIGGER_MODE_SHIFT 2

#define BUS_TYPE_ISA 0
#define BUS_TYPE_PCI 1

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
    u8 overriden;
    u8 polarity;
    u8 trigger;
};

struct bus {
    u8 id;
    u8 type;
};

struct ioapic_entry {
    union {
        struct {
            u32 low;
            u32 high;
        }__attribute__((packed));
        struct {
            u8 vector;
            u8 delivery_mode:4;
            u8 destination_mode:1;
            u8 destination_status:1;
            u8 polarity:1;
            u8 remote_irr:1;
            u8 trigger:1;
            u8 mask:1;
            u16 pad1:14;
            u32 pad2:24;
            u8 destination;
        }__attribute__((packed));
    }__attribute__((packed));
}__attribute__((packed));

void interrupts_init();
struct ioapic *ioapic_allocate(u8 ioapic_id, u32 base_register, u32 gsi_base);
void interrupt_set_override(u8 bus, u8 source, u32 gsi, u8 polarity, u8 trigger);
void interrupts_enable();
void ioapic_init();

#endif
