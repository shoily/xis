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

void itoa(int val, char *str, int base) {
    char *p = str;
    int rem;
    char temp;
    unsigned int value = (unsigned int)val;

    if (base == 10 && val < 0) {
        value = (unsigned int)-val;
        *p++ = '-';
        str++;
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

void ltoa(long val, char *str, int base) {

#ifdef X86_64
	lltoa((long long)val, str, base);
#else
	itoa((int)val, str, base);
#endif
}

void ptrtoa(void *val, char *str, bool maxfill) {

	char *s, *d;
	int len, idx;
	int pass = 0;
	int first_pass_idx;
	int total_len;

	while (pass < 2) {

		if (pass == 0) {
#ifdef X86_64
			itoa((int)val >> 32, str, 16);
#else
			itoa((int)val, str, 16);
#endif
			if (maxfill) {
				first_pass_idx = 8;
			} else {
				first_pass_idx = strlen(str);
			}
			total_len = first_pass_idx;
			s = str;
		}
#ifdef X86_64
		else {
			s = str + first_pass_idx;
			itoa(val & 0xffffffff, s, 16);
			if (max_fill) {
				total_len += 8;
			} else {
				total_len += strlen(s);
			}
		}
#else
		if (pass > 0)
			break;
#endif

		if (maxfill) {

			len = strlen(s);
			if (len < 8) {
				d = s + 7;
				s += (len - 1);

				idx = 8;
				while (idx >(8 - len)) {
					*d-- = *s--;
					idx--;
				}

				s++;
				while ((8-len) > 0) {
					*s++ = '0';
					len++;
				}
			}
		}

		pass++;
	}

	str[total_len] = 0;
	
}

void lltoa(long long val, char *str, int base) {
	char *s, *d;
	int len, idx;
	int pass = 0;

	if (base == 10 && val < 0) {
		str[0] = 0;
		return;
	}

	while (pass < 2) {

		if (pass == 0) {
			itoa(val >> 32, str, base);
			s = str;
		}
		else {
			itoa(val & 0xffffffff, str + 8, base);
			s = str + 8;
		}

		len = strlen(s);
		if (len < 8) {
			d = s + 7;
			s += (len - 1);

			idx = 8;
			while (idx >(8 - len)) {
				*d-- = *s--;
				idx--;
			}

			s++;
			while ((8-len) > 0) {
				*s++ = '0';
				len++;
			}
		}

		pass++;
	}

	str[16] = 0;
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
		char *s;
		void *p;
	} val;
	char str[20];

	while(*p) {

		if (*p == '%') {
			p++;
			if (*p == 37 || *p == '\0') {

				*buf++ = 37;
			} else {

				if (*p == 'c') {

					val.c = va_arg(list, char);
					*buf++ = val.c;
				} else if (*p == 'i' || *p == 'd' || *p == 'x') {

					val.i = va_arg(list, int);
					itoa(val.i, str, (*p == 'x') ? 16 : 10);
					ptr = str;
					while(*ptr) {
						*buf++=*ptr++;
					}
				} else if (*p == 'p') {

					val.p = va_arg(list, void*);
					ptrtoa(val.p, str, 16);
					ptr = str;
					while(*ptr) {
						*buf++=*ptr++;
					}
				} else if (*p == 'u') {

					val.u = va_arg(list, unsigned int);
					lltoa(val.u, str, 10);
					ptr = str;
					while(*ptr) {
						*buf++=*ptr++;
					}
				} else if (*p == 'l' && *(p+1) == 'l') {

					p++;
					val.ll = va_arg(list, long long);
					lltoa(val.ll,str, 10);
					ptr = str;
					while(*ptr) {
						*buf++=*ptr++;
					}
				} else if (*p == 's') {

					ptr = val.s = va_arg(list, char*);
					while(*ptr) {
						*buf++=*ptr++;
					}
				}
			}
		} else {

			*buf++=*p;
		}

		p++;
	}

	*buf='\0';
}
