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
#include "common.h"
#include "x86.h"
#include "x86_32.h"

//
//  Start kernel routine
//

int start_kernel(void) {

    print_vga("XIS kernel started (v1.0)", true);
    print_vga("", true);
    dump_e820();
    start32();

    return 0;
}
