/*****************************************************************************/
/*  File: util.h                                                             */
/*                                                                           */
/*  Description: Source file for utility functions                           */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Mar 15, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#include "util.h"
#include "type.h"
#include "system.h"

int strlen(char *p) {
    int len = 0;
    
    while(*p++) {
        len++;
    }

    return len;
}

void _itoa(int val, char *str, int base, bool not_signed) {
    char *p = str;
    int rem;
    char temp;
    unsigned int value = (unsigned int)val;

    if (!(base == 10 || base == 16))
        return;

    if (base !=16 && !not_signed && val < 0) {
        if (val < 0) {
            if (base == 10) {
                value = (unsigned int)-val;
                *p++ = '-';
                str++;
            }
        }
    }

    do {
        rem = value % base;
        value /= base;

        if (base == 10) {
            *p = rem + '0';
        } else if (base == 16) {

            if (rem < 10) {
                *p = rem + '0';
            } else {
                *p = rem - 10 + 'A';
            }
        }

        p++;
        
    } while (value);

    *p-- = 0;
    while (p > str) {
        temp = *p;
        *p = *str;
        *str = temp;
        str++;
        p--;
    }
}

void _lltoa(long long value, char *str, int base, bool maxfill) {
    char *p = str;
    int upper_dword = 0;
    int len;
    char *tmp;

    if (!(base == 10 || base == 16))
        return;

#ifndef CONFIG_64
    if (base == 10)
        return;
#endif

    if (base != 16 && value < 0) {
        if (base == 10) { 
            value = -value;
            *p++ = '-';
            str++;
        }
    }

    upper_dword = (value >> 32);
    if (upper_dword != 0) {
        _itoa(upper_dword, p, base, true);
        p += strlen(p);
    }

    _itoa((int)(value & 0xffffffff), p, base, true);
    if (upper_dword && strlen(p) != 8) {
        len = strlen(p);
        tmp = p + 7;
        p += len - 1;
        *(tmp+1) = 0;
        while(len--)
            *tmp-- = *p--;
        len = 8 - strlen(tmp + 1);
        p++;
        while(len--) {
            *p++ = '0';
        }
    }

    if (maxfill && base == 16) {
        p = str;
        len = strlen(p);
        if (len < 16) {
            tmp = p + 15;
            p += len - 1;
            *(tmp+1) = 0;
            while(len--)
                *tmp-- = *p--;
            len = 16 - strlen(tmp + 1);
            p++;
            while(len--)
                *p++ = '0';
        }
    }
}

void ptrtoa(void *val, char *str) {
    int len;
    char *p, *tmp;

#ifdef CONFIG_64
    ltoa(val, str, 16);
#else
    _itoa((int)val, str, 16, true);
#endif

    p = str;
    len = strlen(p);
    if (len < ((int)sizeof(long) * 2)) {
        tmp = p + (sizeof(long) * 2) - 1;
        p += len - 1;
        *(tmp+1) = 0;
        while(len--)
            *tmp-- = *p--;
        len = (sizeof(long) * 2) - strlen(tmp + 1);
        p++;
        while(len--)
            *p++ = '0';
    }
}

int strncmp(const char *s1, const char *s2, size_t len) {

    for(size_t i=0;i<len;i++) {

        if (*s1 < *s2)
            return -1;
        else if (*s1 > *s2)
            return 1;

        s1++; s2++;
    }

    return 0;
}

void sprintf(char *buf, char *fmt, ...) {
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

    while(*p) {
        if (*p == '%') {
            p++;
            if (*p == 37 || *p == '\0') {
                *buf++ = 37;
            } else if (*p == 'c') {
                val.c = va_arg(list, char);
                *buf++ = val.c;
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

                while(*ptr)
                    *buf++=*ptr++;
            }
        } else {
            *buf++=*p;
        }
        p++;
    }
    *buf='\0';
}

int ring_buffer_init(struct ring_buffer *rb, void *buffer, u16 buffer_size) {
    if (!buffer_size)
        return -1;
    memset(buffer, 0, buffer_size);
    rb->buffer_size = buffer_size;
    rb->buffer = buffer;
    rb->cur_ptr = 0;
    rb->len = 0;
    return 0;
}

int ring_buffer_put_elem(struct ring_buffer *rb, u8 elem) {
    if (((rb->cur_ptr + rb->len + 1) % rb->buffer_size) == rb->cur_ptr)
        return -1;
    rb->buffer[rb->cur_ptr] = elem;
    rb->len = rb->len + 1;
    return 0;
}

int ring_buffer_get_elem(struct ring_buffer *rb, u8 *elem) {
    if (!rb->len || !elem)
        return -1;

    *elem = rb->buffer[rb->cur_ptr];
    rb->cur_ptr = (rb->cur_ptr + 1) % rb->buffer_size;
    rb->len--;
    return 0;
}
