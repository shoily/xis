/*****************************************************************************/
/*  File: debug.h                                                            */
/*                                                                           */
/*  Description: Header file for debugging support.                          */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Nov 6, 2020                                                        */
/*                                                                           */
/*****************************************************************************/

#ifndef _DEBUG_H
#define _DEBUG_H

#define KERNEL_DEBUG 1
#define KERNEL_INFO 2
#define KERNEL_WARN 3
#define KERNEL_ERR 4
#define KERNEL_CRIT 5

void debug_init();
void printf(int level, char *fmt, ...);
//void debug_msg(char *msg, int len);

#endif
