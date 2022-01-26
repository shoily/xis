/*****************************************************************************/
/*  File: start.c                                                            */
/*                                                                           */
/*  Description: Kernel initialization code.                                 */
/*  start_kernel routine is called from boot32.S.                            */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Feb 11, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#include "type.h"
#include "util.h"
#include "system.h"
#include "setup32.h"
#include "memory.h"
#include "usermode.h"
#include "smp.h"
#include "acpi.h"
#include "debug.h"
#include "interrupt.h"
#include "lock.h"

//
//  Start kernel routine
//

extern spinlock spinlock_smp;

int start_kernel(void) {

	vga_init();
    INIT_SPIN_LOCK(&spinlock_smp);
    pgd_lock_init();
    if(mem_init())
        return -1;
	debug_init();
    printf(KERNEL_INFO, "XIS kernel started (v1.0)\n\n");
    dump_e820();
    setup32();
	bda_read_table();
    interrupts_init();
	acpi_init();
	ioapic_init();
	smp_start();
	initialize_usermode();
    printf(KERNEL_INFO, "Kernel started");
    switch_to_um();

    return 0;
}
