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
#include "util.h"
#include "system.h"

extern int ebda;
extern int ioapic_base_register;
extern int ioapic_gsi_base;

typedef struct _RSD {

	char signature[8];
	u8 checksum;
	char OEMID[6];
	u8 revision;
	u32 rsdt_address;
}__attribute__((packed)) RSD;

typedef struct _ACPI_TABLE {

	char signature[4];
	u32 length;
	u8 revision;
	u8 checksum;
	char OEMID[6];
	char OEM_table_id[8];
	u32 OEM_revision;
	u32 creator_id;
	u32 creator_revision;
}__attribute__((packed)) ACPI_TABLE;

typedef struct _APIC_TABLE {

	char type;
	char length;

	union {
		struct {
			u8 uid;
			u8 apic_id;
			u32 flags;
		}__attribute__((packed)) processor_lapic;

		struct {
			u8 id;
			u8 reserved;
			u32 address;
			u32 gsi_base;
		}__attribute__((packed)) ioapic;

		struct {
			u8 bus;
			u8 source;
			u32 gsi;
			u16 flags;
		}__attribute__((packed)) int_src_override;

		struct {
			u16 flags;
			u32 gsi;
		}__attribute__((packed)) nmi_source;

		struct {
			u8 uid;
			u16 flags;
			u8 lapic_lint;
		}__attribute__((packed)) lapic_nmi;
	} info;
	
}__attribute__((packed)) APIC_TABLE;

#define RSDPTR_STR "RSD PTR"
#define APIC_STR "APIC"

RSD *rsdp;
ACPI_TABLE *rsdt;
ACPI_TABLE **acpi_tables;

int num_acpi_tables;

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

		if (!memcmp(cur->signature, RSDPTR_STR, strlen(RSDPTR_STR))) {

			rsdp = cur;
			printf(KERNEL_INFO, "RSDP: %p\n", (long)rsdp-KERNEL_VIRT_ADDR);
			return true;
		}

		cur++;
	}

	return false;
}

void acpi_process_madt(ACPI_TABLE *madt) {

	int remaining_bytes;
	APIC_TABLE *apic_table;

	remaining_bytes = madt->length - 44;
	apic_table = (APIC_TABLE*)ADD2PTR(madt, 44);

	do { 

		remaining_bytes -= apic_table->length;

		switch(apic_table->type) {

		case 0:
		{
			// local APICs
		} break;
		case 1:
		{
			// currently supports one IOAPIC
			if (!ioapic_base_register) {
				ioapic_base_register = (int)apic_table->info.ioapic.address;
				ioapic_gsi_base = (u32)apic_table->info.ioapic.gsi_base;
				printf(KERNEL_INFO, "IOAPIC base register: %p\n", ioapic_base_register);
			}
		} break;
		case 2:
		{
			// interrupt source override
		} break;
		case 3:
		{
			// NMI source
		} break;
		case 4:
		{
			// local APIC NMI
		} break;

		}

		apic_table = (APIC_TABLE*)ADD2PTR(apic_table, apic_table->length);
	} while(remaining_bytes);
}

void acpi_process_table(ACPI_TABLE *acpi_table) {

	if (!memcmp(acpi_table->signature, APIC_STR, strlen(APIC_STR))) {

		acpi_process_madt(acpi_table);
	}
}

void acpi_init() {

	rsdp = NULL;
	acpi_tables = NULL;
	ioapic_base_register = 0;

	if (!acpi_find_rsdp()) {
	
		printf(KERNEL_INFO, "No ACPI!!!\n");
		return;
	}
	
	rsdt = (ACPI_TABLE*)ADD2PTR(rsdp->rsdt_address, KERNEL_VIRT_ADDR);
	printf(KERNEL_INFO, "RSDT: %p\n", (long)rsdt-KERNEL_VIRT_ADDR);

	num_acpi_tables = (rsdt->length - sizeof(ACPI_TABLE)) / sizeof(void*);
	acpi_tables = (ACPI_TABLE**)ADD2PTR(rsdt, sizeof(ACPI_TABLE));

	for(int i=0;i<num_acpi_tables;i++) {
		acpi_process_table((ACPI_TABLE*)ADD2PTR(acpi_tables[i], KERNEL_VIRT_ADDR));
	}
}
