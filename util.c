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

    if (val < 0) {
        val = -val;
        *p++ = '-';
        str++;
    }

    do {
        
        rem = val % base;
        val /= base;

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
        
    } while (val);

    *p-- = 0;
    while (p > str) {
        temp = *p;
        *p = *str;
        *str = temp;
        str++;
        p--;
    }
}

void itoll(long long val, char *str, int base) {
	char *s, *d;
	int len, idx;
	int pass = 0;

	if (val < 0) {
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
