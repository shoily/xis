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

typedef unsigned char u8;
typedef char s8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;
typedef long long s64;
typedef unsigned long long u64;

typedef u32 size_t;

typedef u64 addr64_t;

#define ALIGN_LONG(size) ((size + sizeof(long) - 1) & ~(sizeof(long) - 1))

#define ADDPTRS(x, y) ((long)(x) + (long)(y))
typedef long addr_t;
#ifdef CONFIG_64
#define ADDPTRS64 ADDPTRS
#define PTR64(x) (x)
#define PTR(x) ((u64)x)
#else
#define ADDPTRS64(x, y) ((u64)(x) + (u64)(y))
#define PTR64(x) ((u64)(u32)(x))
#define PTR(x) ((u32)x)
#endif

#endif
