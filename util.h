/*****************************************************************************/
/*  File: common.h                                                           */
/*                                                                           */
/*  Description: Header file for common/utility functions                    */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Mar 15, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#ifndef _UTIL_H
#define _UTIL_H

#include "type.h"

#define memset(address, size, c) {for(unsigned int i=0;i<size;i++) ((char*)address)[i]=c;}

#define UNUSED(x) (void)(x)

int strlen(char *p);
void itoa(int val, char *str, int base);
void lltoa(long long val, char *str, int base);
void print_msg(char *msg, int info, int base, bool newline);

#endif // _COMMON_H
