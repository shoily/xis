/*****************************************************************************/
/*  File: mp_init.S                                                          */
/*                                                                           */
/*  Description: Startup code for AP.                                        */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Aug 9, 2020                                                        */
/*                                                                           */
/*****************************************************************************/

    .equ AP_COUNT_PHYS_ADDR, 0xfff8
    .code16
    
    .globl init_ap

    .section "mp_init", "ax"

init_ap:

    cli
    xorw %ax, %ax
    movw %ax, %ss
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    incw AP_COUNT_PHYS_ADDR

    hlt
