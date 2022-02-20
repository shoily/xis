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
#include "debug.h"
#include "page32.h"

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
struct tss_entry __attribute__((aligned(4096))) tss[MAX_NUM_SMPS];

int sched_tick[MAX_NUM_SMPS];

extern int _kernel_stack_0_start;
extern addr_t _kernel_pg_dir;

extern int lapic_present;
extern bool ioapic_initialized;
extern int lapic_base_register;

void common_trap_handler();
void trap_handler_14();
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

#define DEBUG_TRAP 1

__attribute__((regparm(0))) void trap_handler(struct regs_frame *rf) {

#ifdef DEBUG_TRAP
    printf(KERNEL_DEBUG, "Trap handler\ngs: %x fs: %x es: %x ds: %x code_nr: %x cs: %x eip: %x ss: %x esp: %x ", rf->gs, rf->fs, rf->es, rf->ds, rf->code_nr, rf->cs, rf->eip, rf->ss, rf->esp);

    if (rf->trap_nr == 14) {
        u32 cr2;
        __asm__ __volatile__(
            "mov %%cr2, %%eax;"
            "mov %%eax, %0;"
            : "=m"(cr2)
            :
            : "%eax");

#ifdef DEBUG_USER_MODE_TRAP
        if (rf->eip < KERNEL_VIRT_ADDR) {
            pgd_t *pgdir, *pgd;
            pte_t *pte;

            pgdir = GET_CPU_PGDIR(CUR_CPU);
            pgd = pgdir + ((rf->eip >> PGD_SHIFT) & 0x3ff);
            printf(KERNEL_INFO, " code - pgd: %x, *pgd: %x -- ", pgd, *pgd);
            pte = (pte_t*)ADDPTRS(*pgd & PAGE_MASK, KERNEL_VIRT_ADDR);
            map_kernel_linear_with_pagetable((addr_t)pte, PAGE_SIZE, 0, MAP_LOCAL_CPU);
            pte += ( (rf->eip >> PAGE_SHIFT) & 0x3ff);
            printf(KERNEL_INFO, " pte: %x, *pte: %x -- ", pte, *pte);
            pgd = pgdir + ((rf->esp >> PGD_SHIFT) & 0x3ff);
            printf(KERNEL_INFO, " stack - pgd: %x, *pgd: %x -- ", pgd, *pgd);
            pte = (pte_t*)ADDPTRS(*pgd & PAGE_MASK, KERNEL_VIRT_ADDR);
            map_kernel_linear_with_pagetable((addr_t)pte, PAGE_SIZE, 0, MAP_LOCAL_CPU);
            pte += ((rf->esp >> PAGE_SHIFT) & 0x3ff);
            printf(KERNEL_INFO, " pte: %x, *pte: %x -- ", pte, *pte);
        }
#endif
        printf(KERNEL_DEBUG, " CR2 reg: %x\n", cr2);
    } else {
        printf(KERNEL_DEBUG, "\n");
    }
#else
	UNUSED(rf);
#endif

    __asm__ __volatile__("1: hlt; jmp 1b;" : : : );
}

int lapic_calibration_tick = 0;
bool lapic_calibration_mode = false;
bool lapic_timer_enabled = false;

#define DEBUG_TIMER

#ifdef DEBUG_TIMER
int timer_counter[MAX_NUM_SMPS];
int seconds[MAX_NUM_SMPS];
#endif

void sendEOI(int intno) {

	if (ioapic_initialized || (intno == 0 && lapic_present)) {

		__asm__ __volatile__("movl %0,%%eax;"
							 "addl $0xb0,%%eax;"
							 "movl $0,(%%eax);"
							 :
							 : "m"(lapic_base_register)
							 : "%eax"
							);
	} else {

		__asm__ __volatile__("movb $0x20,%%al;"
							 "movb %0,%%ah;"
							 "cmpb $8,%%ah;"
							 "jl 1f;"
							 "outb %%al,$0xa0;"
							 "1:"
							 "outb %%al,$0x20;"
							 :
							 : "m"(intno)
							 : "%eax"
							);
	}
}

__attribute__((regparm(0))) void common_interrupt_handler(struct regs_frame *rf) {

    if (rf->code_nr == 0) {

		sched_tick[CUR_CPU]++;

        if (lapic_calibration_mode) {

            lapic_calibration_tick++;
            
        } else {
#ifdef DEBUG_TIMER
			char s[20];
			timer_counter[CUR_CPU]++;
			if ((!lapic_present && timer_counter[CUR_CPU] >= PIT_HZ) ||
				(lapic_present && timer_counter[CUR_CPU] >= (1000*lapic_calibration_tick))) {
				timer_counter[CUR_CPU] = 0;
				seconds[CUR_CPU]++;

				itoa(seconds[CUR_CPU], s, 10);
				print_vga_fixed(s, 140, 5+CUR_CPU);
			}
#endif
		}
    }

	sendEOI(rf->code_nr);
}

__attribute__((regparm(0))) void common_sys_call_handler(struct regs_frame *rf) {

#ifdef DEBUG_SYSCALL
    printf(KERNEL_DEBUG, "System call: %x\n", rf->code_nr | CUR_CPU, 16);
#else
	UNUSED(rf);
#endif
}

// Initializes LDT for user code and data segment selectors
void initializeLDT32() {

    // setup LDT data structure
    memset(ldt, 0, sizeof(ldt));

    SET_USER_CODE_SEGMENT_IN_LDT(ldt, 0xfffff, 0);
    SET_USER_DATA_SEGMENT_IN_LDT(ldt, 0xfffff, 0);

    SET_LDT_DESCRIPTOR(gdt, (sizeof(ldt)), (unsigned int)ldt);
}

void loadLDT32() {

	__asm__ __volatile__("lldt %w0;"
                         :
                         : "q" (LDT_SELECTOR)
                         :
                         );
}

//
// Re-initializes GDT for kernel code and data segement selectors
//

void initializeGDT32() {

    // setup GDT data structure
    memset(gdt, 0, LAST_SEG);
    gdt_desc.size = LAST_SEG - 1;
    gdt_desc.gdt = gdt;

    SET_KERNEL_CODE_SEGMENT(gdt, 0xfffff, 0);
    SET_KERNEL_DATA_SEGMENT(gdt, 0xfffff, 0);
    
    SET_USER_CODE_SEGMENT(gdt, 0xfffff, 0);
    SET_USER_DATA_SEGMENT(gdt, 0xfffff, 0);
}

void loadGDT32() {

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

void initializeTSS32(int smp_id) {

    // setup TSS data structure

    memset(&tss[smp_id], 0, sizeof(struct tss_entry));

    tss[smp_id].esp0 = ((int)&_kernel_stack_0_start)+(smp_id*KERNEL_STACK_SIZE);
    tss[smp_id].ss0 = KERNEL_DATA_SEG;

    SET_TASK_SEGMENT(gdt, (sizeof(struct tss_entry)), (unsigned int)&tss[smp_id], (FIRST_TASK_SEG+(smp_id*8)));
}

void loadTSS32(int smp_id) {

	__asm__ __volatile__("ltr %w0;"
                         :
                         : "q" ((FIRST_TASK_SEG+(smp_id*8)) | 3)
                         :
                         );

}

void initializeIDT32() {

    // setup IDT data structure
    memset(idt, 0, sizeof(idt));
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
    SET_TRAP_GATE(idt, 14, trap_handler_14);

    SET_TRAP_GATE(idt, 128, sys_call_handler_128);
}

void loadIDT32() {

	// load IDT
	__asm__ __volatile__("movl $%0, %%eax;"
                         "lidt (%%eax);"
                         :
                         : "m" (idt_desc)
                         : "%eax"
                         );
}

void set_idt(int vector, idt_function_type idt_function) {

    SET_INTERRUPT_GATE(idt, vector, idt_function);
}

void setup32() {

	memset(sched_tick, 0, sizeof(sched_tick));

    initializeGDT32();
	loadGDT32();
    initializeLDT32();
	loadLDT32();
    initializeIDT32();
	loadIDT32();
    initializeTSS32(0);
	loadTSS32(0);

    mask_pic_8259();
    init_lapic();

    if (!lapic_present) {
        init_pic_8259();
        init_pit_frequency();
    }

    STI;

    printf(KERNEL_INFO, "Setup GDT,IDT, LDT and TSS done\n");
}
