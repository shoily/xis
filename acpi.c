/*****************************************************************************/
/*  File: acpi.c                                                             */
/*                                                                           */
/*  Description: Source file for ACPI.                                       */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Nov 3, 2020                                                        */
/*                                                                           */
/*****************************************************************************/

#include "acpi.h"
#include "debug.h"
#include "krnlconst.h"
#include "debug.h"

extern int ebda;

void *rsd_ptr;

#define CHECK_RSD_PTR(p) (((char*)p)[0]=='R' &&	\
						  ((char*)p)[1]=='S' &&	\
						  ((char*)p)[2]=='D' &&	\
						  ((char*)p)[3]==' ' &&	\
						  ((char*)p)[4]=='P' &&	\
						  ((char*)p)[5]=='T' &&	\
						  ((char*)p)[6]=='R' &&	\
						  ((char*)p)[7]==' ')

void acpi_find_rsdp() {

	int phase = 0;
	int cur, end;

	rsd_ptr = 0;
	
	cur = ebda + KERNEL_VIRT_ADDR;
	end = ebda + 1024;

	while(phase < 2) {

		if (cur == end) {

			phase++;

			if (phase == 1) {

				cur = 0xE0000 + KERNEL_VIRT_ADDR;
				end = cur + 0x20000;
			}
		}

		if (CHECK_RSD_PTR(cur)) {

			rsd_ptr = (void*)cur;
			printf(KERNEL_INFO, "RSD PTR: %p\n", rsd_ptr-KERNEL_VIRT_ADDR);
			break;
		}

		cur += 16;
	}
}
