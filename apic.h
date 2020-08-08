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
#define ISR_0_31 0x100
#define TMR_0_31 0x180
#define IRR_0_31 0x200
#define ERROR_STATUS_REG 0x280
#define ICR_1 0x300
#define ICR_2 0x310
#define LVT_TIMER_REG 0x320
#define LVT_LINT_0_REG 0x350
#define LVT_LINT_1_REG 0x360
#define LVT_ERROR_REG 0x370
#define INITIAL_COUNTER_REG 0x380
#define CURRENT_COUNTER_REG 0x390
#define DIVIDE_CONFIGURATION_REG 0x3e0

void init_lapic();
void lapic_switch(bool enable);

#endif
