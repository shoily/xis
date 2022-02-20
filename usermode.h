/*****************************************************************************/
/*  File: usermode.c                                                         */
/*                                                                           */
/*  Description: Header file for user mode process support.                  */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: July 7, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#ifndef _USERMODE_H
#define _USERMODE_H

// USER_MODE_VIRT_TEXT has to match with the value in um.ld
#define USER_MODE_VIRT_STACK 0x10001000
#define USER_MODE_VIRT_TEXT  0x10004000

void initialize_usermode();
void switch_to_um();

#endif
