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
#include "memory.h"

char* cur_debug_mem;
spinlock lock_debug_mem;
int cur_offset;
char *kernel_debug_buffer;
int debug_level_for_vga;

#define DEBUG_BUFFER_SIZE PAGE_SIZE

void debug_init() {

    kernel_debug_buffer = page_alloc_kmap_linear(DEBUG_BUFFER_SIZE >> PAGE_SHIFT);
    cur_debug_mem = kernel_debug_buffer;
    cur_offset = 0;
    debug_level_for_vga = KERNEL_DEBUG;
    INIT_SPIN_LOCK(&lock_debug_mem);
}

#define WRITE_TO_BUF(addr, ch) { \
        if(addr==(char*)ADDPTRS(kernel_debug_buffer,DEBUG_BUFFER_SIZE-1)) { \
            *++addr='\0';                                                   \
            if (level >= debug_level_for_vga) {                             \
                print_vga(cur_debug_mem);                                   \
            }                                                               \
            cur_debug_mem=addr=kernel_debug_buffer;                         \
        }                                                                   \
        *addr++=(char)ch;                                                   \
    }                                                                       \

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
        long l;
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
                WRITE_TO_BUF(buf, 37);
            } else if (*p == 'c') {
                val.c = va_arg(list, char);
                WRITE_TO_BUF(buf, val.c);
            } else {
                if (*p == 'i' || *p == 'd' || *p == 'x') {
                    val.i = va_arg(list, int);
                    itoa(val.i, str, (*p == 'x') ? 16 : 10);
                    ptr = str;
                } else if (*p == 'p') {
                    val.p = va_arg(list, void*);
                    ptrtoa(val.p, str);
                    ptr = str;
                } else if (*p == 'u') {
                    val.u = va_arg(list, unsigned int);
                    _itoa(val.u, str, 10, true);
                    ptr = str;
                } else if (*p == 'l' && *(p+1) == 'l') {
                    p++;
                    val.ll = va_arg(list, long long);
                    if (*(p+1) == 'x') {
                        p++;
                        _lltoa(val.ll, str, 16, true);
                    } else {
                        lltoa(val.ll, str, 10);
                    }
                    ptr = str;
                } else if (*p == 'l') {
                    val.l = va_arg(list, long);
                    ltoa(val.l, str, (*(p+1) == 'x') ? 16 : 10);
                    if (*(p+1) == 'x')
                        p++;
                    ptr = str;
                } else if (*p == 's') {
                    ptr = val.s = va_arg(list, char*);
                }

                while(*ptr) {
                    WRITE_TO_BUF(buf, *ptr);
                    ptr++;
                }
            }
        } else {
            WRITE_TO_BUF(buf, *p);
        }
        p++;
    }

    WRITE_TO_BUF(buf, '\0');

    if (level >= debug_level_for_vga) {
        print_vga(cur_debug_mem);
    }

    cur_debug_mem = buf;

    spinlock_unlock(&lock_debug_mem);
}
