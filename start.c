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

//
//  Start kernel routine
//

int start_kernel(void) {

	vga_init();
    print_vga("XIS kernel started (v1.0)", true);
    print_vga("", true);
    dump_e820();
    setup32();
    init_memory();
	usermode_load_first_program();
	smp_start();
	initialize_usermode();
    switch_to_um();

    return 0;
}
