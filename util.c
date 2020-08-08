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

void lltoa(long long val, char *str, int base) {
	char *s, *d;
	int len, idx;
	int pass = 0;

	if (base ==10 && val < 0) {
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

void print_msg(char *msg, int info, int base, bool newline) {

    char str[20];

    print_vga(msg, false);
    print_vga(": ", false);
    itoa((unsigned int)info, str, base);
    print_vga(str, newline);
    if (!newline) {
        print_vga(" ", false);
    }
}
