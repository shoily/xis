/*****************************************************************************/
/*  File: debug.c                                                            */
/*                                                                           */
/*  Description: Source file for debugging support.                          */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Nov 6, 2020                                                        */
/*                                                                           */
/*****************************************************************************/

#include "debug.h"
#include "lock.h"
#include "util.h"
#include "system.h"

char* cur_debug_mem;
spinlock lock_debug_mem;
int cur_offset;
extern int _kernel_debug_buffer;
int debug_level_for_vga;

void debug_init() {

	cur_debug_mem = (char *)&_kernel_debug_buffer;
	cur_offset = 0;
	debug_level_for_vga = KERNEL_DEBUG;
	INIT_SPIN_LOCK(&lock_debug_mem);
}

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

void printf(int level, char *fmt, ...) {

	va_list list;
	va_start(fmt, list);
	char *p = fmt;
	char *ptr;

	union {
		char c;
		int i;
		unsigned int u;
		long long ll;
		char *s;
		void *p;
	} val;
	char str[20];
	char *buf;

	spinlock_lock(&lock_debug_mem);
	buf = cur_debug_mem;

	while(*p) {

		if (*p == '%') {
			p++;
			if (*p == 37 || *p == '\0') {

				*buf++ = 37;
				//if (*p != '\0')
				//	p++;
			} else {

				if (*p == 'c') {

					val.c = va_arg(list, char);
					*buf++ = val.c;
				} else if (*p == 'i' || *p == 'd' || *p == 'x') {

					val.i = va_arg(list, int);
					itoa(val.i, str, (*p == 'x') ? 16 : 10);
					ptr = str;
					while(*ptr) {
						*buf++ = *ptr++;
					}
				} else if (*p == 'p') {

					val.p = va_arg(list, void*);
					ptrtoa(val.p, str, 16);
					ptr = str;
					while(*ptr) {
						*buf++ = *ptr++;
					}
				} else if (*p == 'u') {

					val.u = va_arg(list, unsigned int);
					lltoa(val.u, str, 10);
					ptr = str;
					while(*ptr) {
						*buf++ = *ptr++;
					}
				} else if (*p == 'l' && *(p+1) == 'l') {

					p++;
					val.ll = va_arg(list, long long);
					lltoa(val.ll,str, 10);
					ptr = str;
					while(*ptr) {
						*buf++ = *ptr++;
					}
				} else if (*p == 's') {

					ptr = val.s = va_arg(list, char*);
					while(*ptr) {
						*buf++ = *ptr++;
					}
				}
			}
		} else {

			*buf++ = *p;
		}

		p++;
	}

	*buf++ = '\0';

	if (level >= debug_level_for_vga) {

		print_vga(cur_debug_mem);
	}

	cur_debug_mem = buf;

	spinlock_unlock(&lock_debug_mem);
}
/*
void debug_msg(int flags, char *msg, int len) {

	int i = 0;
	
	spinlock_lock(&lock_debug_mem);

	while (i < len) {

		*cur_debug_mem++ = msg[i++];
	}

	*cur_debug_mem++ = 0;
	*cur_debug_mem++ = 0;

	spinlock_unlock(&lock_debug_mem);
}
*/
