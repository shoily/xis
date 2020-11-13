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

//
//  Start kernel routine
//

int start_kernel(void) {

	vga_init();
	debug_init();
    printf(KERNEL_INFO, "XIS kernel started (v1.0)\n\n");
    dump_e820();
    setup32();
    init_memory();
	bda_read_table();
	//acpi_find_rsdp();
	usermode_load_first_program();
	smp_start();
	initialize_usermode();
    switch_to_um();

    return 0;
}
