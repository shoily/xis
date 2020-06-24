/*****************************************************************************/
/*  File: setup32.h                                                          */
/*                                                                           */
/*  Description: Header file for x86 32 bit data structure for GDT, IDT, TSS */
/*  and related macros.                                                      */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Mar 15, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#ifndef _SETUP_32_H
#define _SETUP_32_H

#define USER_CODE_SEG 0x10
#define USER_DATA_SEG 0x18
#define KERNEL_CODE_SEG 0x20
#define KERNEL_DATA_SEG 0x28
#define TASK_SEG 0x30
#define LAST_SEG (TASK_SEG+8)

struct gdt_entry {
    unsigned short segment_limit_low;
    unsigned short base_address_low;
    unsigned char base_address_mid;

    unsigned int type:4;
    unsigned int system:1;
    unsigned int dpl:2;
    unsigned int present:1;
  
    unsigned char segment_limit_high:4;

    unsigned int avl:1;
    unsigned int l:1;
    unsigned int db:1;
    unsigned int granularity:1;
  
    unsigned char base_address_high;
}__attribute__((packed));

struct idt_entry {
    unsigned short offset_low;
    unsigned short segment_selector;

    unsigned char unused;
    unsigned char type:4;
    unsigned char s:1;
    unsigned char dpl:2;
    unsigned char present:1;

    unsigned short offset_high;
}__attribute__((packed));

struct tss_entry {
    unsigned short prev_task;
    int reserved1:16;
    unsigned int esp0;
    int reserved2:16;
    unsigned short ss0;
    unsigned int esp1;
    int reserved3:16;
    unsigned short ss1;
    unsigned int esp2;
    int reserved4:16;
    unsigned short ss2;
    int reserved5:16;
    unsigned int cr3;
    unsigned int eip;
    unsigned int eflags;
    unsigned int eax;
    unsigned int ecx;
    unsigned int edx;
    unsigned int ebx;
    unsigned int esp;
    unsigned int ebp;
    unsigned int esi;
    unsigned int edi;
    unsigned short es;
    int reserved7:16;
    unsigned short cs;
    int reserved8:16;
    unsigned short ss;
    int reserved9:16;
    unsigned short ds;
    int reserved10:16;
    unsigned short fs;
    int reserved11:16;
    unsigned short gs;
    int reserved12:16;
    unsigned short ldt;
    int reserved13:16;
    int reserved14:16;
    unsigned short ioremapaddr;
}__attribute__((packed));

#define SET_GDT_ENTRY(g, index, segment_limit, base_address, _type, _system, _dpl, _db) { \
        struct gdt_entry *entry = g+index;                              \
        memset(entry, sizeof(struct gdt_entry), 0);                     \
        entry->segment_limit_low = segment_limit & 0xffff;              \
        entry->segment_limit_high = (segment_limit >> 16) & 0xf;        \
        entry->base_address_low = base_address & 0xffff;                \
        entry->base_address_mid = (base_address >> 16) & 0xf;           \
        entry->base_address_high = (base_address >> 24) & 0xff;         \
        entry->type = _type;                                            \
        entry->system = _system;                                        \
        entry->dpl = _dpl;                                              \
        entry->present = 1;                                             \
        entry->avl = 0;                                                 \
        entry->l = 0;                                                   \
        entry->db = _db;                                                \
        entry->granularity = 1;                                         \
    }

#define SET_KERNEL_CODE_SEGMENT(g, segment_limit, base_address) SET_GDT_ENTRY(g, (KERNEL_CODE_SEG/8), segment_limit, base_address, 0xc, 1, 0, 1)
#define SET_KERNEL_DATA_SEGMENT(g, segment_limit, base_address) SET_GDT_ENTRY(g, (KERNEL_DATA_SEG/8), segment_limit, base_address, 2, 1, 0, 1)

#define SET_USER_CODE_SEGMENT(g, segment_limit, base_address) SET_GDT_ENTRY(g, (USER_CODE_SEG/8), segment_limit, base_address, 0xc, 1, 3, 1)
#define SET_USER_DATA_SEGMENT(g, segment_limit, base_address) SET_GDT_ENTRY(g, (USER_DATA_SEG/8), segment_limit, base_address, 2, 1, 3, 1)

#define SET_TASK_SEGMENT(g, segment_limit, base_address) SET_GDT_ENTRY(g, (TASK_SEG/8), segment_limit, base_address, 9, 0, 0, 0)

#define SET_IDT_ENTRY(idt, index, offset, _segment_selector, _type, _dpl, _present) { \
        struct idt_entry *entry = idt+index;                            \
        memset(entry, sizeof(struct idt_entry), 0);                     \
        entry->offset_low = (int)offset & 0xffff;                       \
        entry->offset_high = ((int)offset >> 16) & 0xffff;              \
        entry->segment_selector = _segment_selector;                    \
        entry->type = _type;                                            \
        entry->dpl = _dpl;                                              \
        entry->present = _present;                                      \
    }

#define SET_INTERRUPT_GATE(idt, index, offset) {                        \
        SET_IDT_ENTRY(idt, index, offset, KERNEL_CODE_SEG, 0xe, 0, 1);  \
    }

#define SET_TRAP_GATE(idt, index, offset) {                             \
        SET_IDT_ENTRY(idt, index, offset, KERNEL_CODE_SEG, 0xf, 0, 1);  \
    }

#define SET_TASK_GATE(idt, index) {                                 \
        SET_IDT_ENTRY(idt, index, 0, KERNEL_CODE_SEG, 0x5, 0, 1);   \
    }

void setup32();

#endif