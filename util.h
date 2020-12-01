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

#define memset(address, size, c) {for(int i=0;i<(int)size;i++) ((char*)address)[i]=c;}
#define memcpy(dest, src, size) {for(int i=0;i<size;i++) ((char*)dest)[i]=((char*)src)[i];}

#define UNUSED(x) (void)(x)

typedef unsigned char *va_list;
#define va_start(arg, list) (list = (unsigned char*)&arg+sizeof(arg))
#define va_arg(list, type) *((type *) ((list+=sizeof(type))-sizeof(type)))

int strncmp(const char *s1, const char *s2, size_t len);
int strlen(char *p);

#define memcmp(p1, p2, num) strncmp((const char *)p1, (const char *)p2, num)

void itoa(int val, char *str, int base);
void ltoa(long val, char *str, int base);
void lltoa(long long val, char *str, int base);
void ptrtoa(void *val, char *str, bool maxfill);

#endif // _COMMON_H
