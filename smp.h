/*****************************************************************************/
/*  File: smp.h                                                              */
/*                                                                           */
/*  Description: Header file SMP related code.                               */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Aug 9, 2020                                                        */
/*                                                                           */
/*****************************************************************************/

#ifndef SMP_H
#define SMP_H

#define AP_INIT_PHYS_TEXT 0x7c00

//#define DEFINE_PER_CPU_VAR(type, var) type (per_cpu_)##(var)[MAX_NUM_SMPS]

void smp_start();

#endif
