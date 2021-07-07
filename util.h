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
void sprintf(char *buf, char *fmt, ...);

typedef enum _va_type {

	VA_BYTE = 0,
	VA_PERCENT = 1,
	VA_CHAR = 2,
	VA_UCHAR = 3,
	VA_INT = 4,
	VA_UINT = 5,
	VA_HEX = 6,
	VA_SHORT = 7,
	VA_USHORT = 8
} va_type;

#endif // _UTIL_H
