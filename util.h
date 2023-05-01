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

#define memset(address, c, size) {for(u32 i=0;i<(size);i++) ((char*)address)[i]=c;}
#define memcpy(dest, src, size) {for(u32 i=0;i<(size);i++) ((char*)dest)[i]=((char*)src)[i];}

#define UNUSED(x) (void)(x)

typedef unsigned char *va_list;
#define va_start(arg, list) (list = (unsigned char*)&arg+sizeof(arg))
#define va_arg(list, type) *((type *) ((list+=sizeof(type))-sizeof(type)))

int strncmp(const char *s1, const char *s2, size_t len);
int strlen(char *p);

#define memcmp(p1, p2, num) strncmp((const char *)p1, (const char *)p2, num)

void _itoa(int val, char *str, int base, bool not_signed);
void _lltoa(long long value, char *str, int base, bool maxfill);
void ptrtoa(void *val, char *str);
void sprintf(char *buf, char *fmt, ...);

#ifdef CONFIG_64
#define ltoa(v,s,b) lltoa((long)v,s,b)
#else
#define ltoa(v,s,b) itoa((int)v,s,b)
#endif

#define itoa(v,s,b) _itoa(v,s,b,false)
#define lltoa(v,s,b) _lltoa(v,s,b,false)
#define ptrtoa64(v,s) _lltoa(v,s,16,sizeof(long long),true)

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

struct list {
    struct list *prev;
    struct list *next;
};

#define list_init(l)  ((l)->prev = (l)->next = l)
#define list_insert_tail(h,l) {(l)->next = h; (l)->prev = (h)->prev; (h)->prev->next = (l); (h)->prev = (l);}
#define list_next_entry(l) (list_empty(l) ? NULL : (l)->next)
#define list_prev_entry(l) (list_empty(l) ? NULL : (l)->prev)
#define list_empty(l) ((l)->next == (l)->prev)
#define list_remove_entry(l) {if (!list_empty(l)) { (l)->next->prev = (l)->prev; (l)->prev->next = (l)->next; }}
#define list_for_each_entry(l,e) for(e=(l)->next;e!=(l);e=e->next)

#define offset_of(s,m) (&((s *)(0))->m)
#define container_of(l,s,m) ((s*)((long)l-(long)offset_of(s,m)))

struct ring_buffer {
    u8 *buffer;
    u16 buffer_size;
    u16 cur_ptr;
    u16 len;
};

int ring_buffer_init(struct ring_buffer *rb, void *buffer, u16 buffer_size);
int ring_buffer_put_elem(struct ring_buffer *rb, u8 elem);
int ring_buffer_get_elem(struct ring_buffer *rb, u8 *elem);

#endif // _UTIL_H
