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
#include "type.h"

extern int ebda;

typedef struct _RSD {

	char signature[8];
	byte checksum;
	char OEMID[6];
	byte revision;
	int rsdt_address;
}__attribute__((packed)) RSD;

#define CHECK_RSD_PTR(p) (p[0]=='R' &&	\
						  p[1]=='S' &&	\
						  p[2]=='D' &&	\
						  p[3]==' ' &&	\
						  p[4]=='P' &&	\
						  p[5]=='T' &&	\
						  p[6]=='R' &&	\
						  p[7]==' ')

RSD *rsd;

bool acpi_find_rsdp() {

	int phase = 0;
	RSD *cur, *end;

	cur = (RSD*)ADD2PTR(ebda, KERNEL_VIRT_ADDR);
	end = (RSD*)ADD2PTR(ebda, 1024);

	while(phase < 2) {

		if (cur == end) {

			phase++;

			if (phase == 1) {

				cur = (RSD*)ADD2PTR(0xE0000, KERNEL_VIRT_ADDR);
				end = (RSD*)ADD2PTR(cur, 0x20000);
			}
		}

		if (CHECK_RSD_PTR(cur->signature)) {

			rsd = cur;
			printf(KERNEL_INFO, "RSD PTR: %p\n", (long)rsd-KERNEL_VIRT_ADDR);
			return true;
		}

		cur++;
	}

	return false;
}

void acpi_init() {

	rsd = NULL;
	if (!acpi_find_rsdp()) {
	
		printf(KERNEL_INFO, "No ACPI!!!\n");
		return;
	}
	printf(KERNEL_INFO, "RSDT: %p\n", rsd->rsdt_address);
}
