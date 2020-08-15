/*****************************************************************************/
/*  File: setup32.c                                                          */
/*                                                                           */
/*  Description: x86 32 bit GDT re-initialization code to enable IDT, LDT    */
/*  and TSS                                                                  */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Mar 25, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#include "util.h"
#include "system.h"
#include "setup32.h"
#include "apic.h"
#include "smp.h"

// GDT data
struct gdt_entry __attribute__((aligned(8))) gdt[LAST_SEG/8];

struct _gdt_desc {
    unsigned short size;
    struct gdt_entry *gdt;
}__attribute__((packed));

struct _gdt_desc __attribute__((aligned(8))) gdt_desc;

// LDT data
struct gdt_entry __attribute__((aligned(8))) ldt[2];

// IDT data
struct idt_entry __attribute__((aligned(8))) idt[256];

struct _idt_desc {
    unsigned short size;
    struct idt_entry *idt;
}__attribute__((packed));

struct _idt_desc __attribute__((aligned(8))) idt_desc;

// TSS data
struct tss_entry __attribute__((aligned(4096))) tss;

int sched_tick = 0;

extern int _kernel_stack_0_start;
extern int _kernel_pg_dir;

extern int lapic_present;

void common_trap_handler();
void sys_call_handler_128();

void irq_handler_0();
void irq_handler_1();
void irq_handler_2();
void irq_handler_3();
void irq_handler_4();
void irq_handler_5();
void irq_handler_6();
void irq_handler_7();
void irq_handler_8();
void irq_handler_9();
void irq_handler_10();
void irq_handler_11();
void irq_handler_12();
void irq_handler_13();
void irq_handler_14();
void irq_handler_15();

void lapic_irq_handler_0();
void lapic_irq_handler_1();

__attribute__((regparm(0))) void trap_handler(struct regs_frame *rf) {

    print_vga("Trap handler", true);
    
    print_msg("gs", rf->gs, 16, false);
    print_msg("fs", rf->fs, 16, false);
    print_msg("es", rf->es, 16, false);
    print_msg("ds", rf->ds, 16, false);
    print_msg("code_nr", rf->code_nr, 16, false);
    print_msg("cs", rf->cs, 16, false);
    print_msg("eip", rf->eip, 16, false);
    print_msg("ss", rf->ss, 16, false);
    print_msg("esp", rf->esp, 16, false);

    __asm__ __volatile__("1: hlt; jmp 1b;" : : : );
}

__attribute__((regparm(0))) void common_interrupt_handler(struct regs_frame *rf) {

    if (rf->code_nr == 0) {

        sched_tick++;
    }
}

__attribute__((regparm(0))) void common_sys_call_handler(struct regs_frame *rf) {

    print_msg("System call", rf->code_nr, 16, true);
}

// Initializes LDT for user code and data segment selectors
void setupLDT32() {

    // setup LDT data structure
    memset(ldt, sizeof(ldt), 0);

    SET_USER_CODE_SEGMENT_IN_LDT(ldt, 0xfffff, 0);
    SET_USER_DATA_SEGMENT_IN_LDT(ldt, 0xfffff, 0);

    SET_LDT_DESCRIPTOR(gdt, (sizeof(ldt)), (unsigned int)ldt);

    MFENCE;

    __asm__ __volatile__("lldt %w0;"
                         :
                         : "q" (LDT_SELECTOR)
                         :
                         );
}

//
// Re-initializes GDT for kernel code and data segement selectors
//

void setupGDT32() {

    // setup GDT data structure
    memset(gdt, LAST_SEG, 0);
    gdt_desc.size = LAST_SEG - 1;
    gdt_desc.gdt = gdt;

    SET_KERNEL_CODE_SEGMENT(gdt, 0xfffff, 0);
    SET_KERNEL_DATA_SEGMENT(gdt, 0xfffff, 0);
    
    SET_USER_CODE_SEGMENT(gdt, 0xfffff, 0);
    SET_USER_DATA_SEGMENT(gdt, 0xfffff, 0);

    MFENCE;

    // Re-initialize GDT
    __asm__ __volatile__("movl $%0, %%eax;"
                         "lgdt (%%eax);"
                         "ljmpl %1, $reinitsegs;"
                         "reinitsegs:;"
                         "movl %2, %%eax;"
                         "movl %%eax, %%ds;"
                         "movl %%eax, %%es;"
                         "movl %%eax, %%ss;"
                         "movl %%eax, %%fs;"
                         "movl %%eax, %%gs;"
                         :
                         : "m" (gdt_desc), "i" (KERNEL_CODE_SEG), "i" (KERNEL_DATA_SEG)
                         : "%eax"
                         );
}

void setupTSS32() {

    // setup TSS data structure

    memset(&tss, sizeof(tss), 0);

    tss.esp0 = (int)&_kernel_stack_0_start;
    tss.ss0 = KERNEL_DATA_SEG;

    SET_TASK_SEGMENT(gdt, (sizeof(tss)), (unsigned int)&tss);

    MFENCE;

    __asm__ __volatile__("ltr %w0;"
                         :
                         : "q" (TASK_SEG | 3)
                         :
                         );
}

void setupIDT32() {

    // setup IDT data structure
    memset(idt, sizeof(idt), 0);
    idt_desc.size = sizeof(idt) - 1;
    idt_desc.idt = idt;

    SET_INTERRUPT_GATE(idt, 32, irq_handler_0);
    SET_INTERRUPT_GATE(idt, 33, irq_handler_1);
    SET_INTERRUPT_GATE(idt, 34, irq_handler_2);
    SET_INTERRUPT_GATE(idt, 35, irq_handler_3);
    SET_INTERRUPT_GATE(idt, 36, irq_handler_4);
    SET_INTERRUPT_GATE(idt, 37, irq_handler_5);
    SET_INTERRUPT_GATE(idt, 38, irq_handler_6);
    SET_INTERRUPT_GATE(idt, 39, irq_handler_7);
    SET_INTERRUPT_GATE(idt, 40, irq_handler_8);
    SET_INTERRUPT_GATE(idt, 41, irq_handler_9);
    SET_INTERRUPT_GATE(idt, 42, irq_handler_10);
    SET_INTERRUPT_GATE(idt, 43, irq_handler_11);
    SET_INTERRUPT_GATE(idt, 44, irq_handler_12);
    SET_INTERRUPT_GATE(idt, 45, irq_handler_13);
    SET_INTERRUPT_GATE(idt, 46, irq_handler_14);
    SET_INTERRUPT_GATE(idt, 47, irq_handler_15);

    SET_TRAP_GATE(idt, 0, common_trap_handler);
    SET_TRAP_GATE(idt, 1, common_trap_handler);
    SET_TRAP_GATE(idt, 2, common_trap_handler);
    SET_TRAP_GATE(idt, 4, common_trap_handler);
    SET_TRAP_GATE(idt, 5, common_trap_handler);
    SET_TRAP_GATE(idt, 6, common_trap_handler);
    SET_TRAP_GATE(idt, 7, common_trap_handler);
    SET_TRAP_GATE(idt, 8, common_trap_handler);
    SET_TRAP_GATE(idt, 9, common_trap_handler);
    SET_TRAP_GATE(idt, 10, common_trap_handler);
    SET_TRAP_GATE(idt, 11, common_trap_handler);
    SET_TRAP_GATE(idt, 12, common_trap_handler);
    SET_TRAP_GATE(idt, 13, common_trap_handler);
    SET_TRAP_GATE(idt, 14, common_trap_handler);

    SET_TRAP_GATE(idt, 128, sys_call_handler_128);

    MFENCE;

    // Initialize IDT
    __asm__ __volatile__("movl $%0, %%eax;"
                         "lidt (%%eax);"
                         :
                         : "m" (idt_desc)
                         : "%eax"
                         );
}

void setup32() {

    setupGDT32();
    setupLDT32();

    init_lapic();

    init_pic_8259();
    init_pit_frequency();

    setupIDT32();
    setupTSS32();

    smp_start();

    STI;

    print_vga("Setup GDT,IDT, LDT and TSS done", true);
}
