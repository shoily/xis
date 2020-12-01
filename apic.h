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

#ifndef APIC_H
#define APIC_H

#include "type.h"

#define LAPIC_ID_REG 0x20
#define LAPIC_VERSION_REG 0x30
#define LAPIC_TPR 0x80
#define LAPIC_APR 0x90
#define LAPIC_PPR 0xa0
#define LAPIC_EOI_REG 0xb0
#define LAPIC_SPURIOUS_REG 0xf0
#define LAPIC_ISR_0_31 0x100
#define LAPIC_TMR_0_31 0x180
#define LAPIC_IRR_0_31 0x200
#define LAPIC_ERROR_STATUS_REG 0x280
#define LAPIC_ICR_1 0x300
#define LAPIC_ICR_2 0x310
#define LAPIC_LVT_TIMER_REG 0x320
#define LAPIC_LVT_LINT_0_REG 0x350
#define LAPIC_LVT_LINT_1_REG 0x360
#define LAPIC_LVT_ERROR_REG 0x370
#define LAPIC_INITIAL_COUNTER_REG 0x380
#define LAPIC_CURRENT_COUNTER_REG 0x390
#define LAPIC_DIVIDE_CONFIGURATION_REG 0x3e0

#define LAPIC_IDT_VECTOR 32
#define LAPIC_DIVIDE_CONFIG_VALUE 10
#define LAPIC_COUNTER_VALUE 1024

void init_lapic();
void lapic_switch(bool enable);
int lapic_read_register(int lapic_register);
void lapic_write_register(int lapic_register, int value);
void lapic_enable_timer();
void ioapic_init();

#endif
