/*****************************************************************************/
/*  File: type.h                                                             */
/*                                                                           */
/*  Description: Data types                                                  */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Mar 24, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#ifndef _TYPE_H
#define _TYPE_H

typedef int bool;
#define true 1
#define false 0

typedef char byte;

#define NULL (void*)0

#define ADD2PTR(x, y) ((long)(x) + (long)(y))

typedef unsigned char u8;
typedef char s8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;
typedef long long s64;
typedef unsigned long long u64;

typedef u32 size_t;

#endif
