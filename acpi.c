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
#include "interrupt.h"
#include "page32.h"

extern int ebda;
extern struct ioapic *ioapic;

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

    u8 type;
    u8 length;

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
            union {
                u16 flags;
                struct {
                    u8 polarity:2;
                    u8 trigger:2;
                    u16 reserved:12;
                }__attribute__((packed));
            }__attribute__((packed));
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
    }__attribute__((packed)) info;
}__attribute__((packed)) APIC_TABLE;

#define RSDPTR_STR "RSD PTR"
#define APIC_STR "APIC"
#define FACP_STR "FACP"

RSD *rsdp;
ACPI_TABLE *rsdt;
ACPI_TABLE **acpi_tables;

int num_acpi_tables;
extern bool keyboard_controller_8042_present;

bool acpi_find_rsdp() {

    int phase = 0;
    char *cur, *end;

    cur = (char*)ADDPTRS(ebda, KERNEL_VIRT_ADDR);
    end = (char*)ADDPTRS(ebda, 1024);

    while(phase < 2) {

        if (cur >= end) {

            phase++;

            if (phase == 1) {

                cur = (char*)ADDPTRS(0xE0000, KERNEL_VIRT_ADDR);
                end = (char*)ADDPTRS(cur, 0x20000);
            }
        }

        if (!strncmp(((RSD*)cur)->signature, RSDPTR_STR, strlen(RSDPTR_STR))) {

            rsdp = (RSD*)cur;
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
    apic_table = (APIC_TABLE*)ADDPTRS(madt, 44);

    do {
        remaining_bytes -= apic_table->length;
        //printf(KERNEL_INFO, "Type: %d, L: %d ", (int)apic_table->type, (int)apic_table->length);

        switch(apic_table->type) {
        case 0:
        {
            //processor_lapic = (APIC_PROCESSOR_LAPIC*)header;
            //printf(KERNEL_INFO, "UID: %d, APIC_ID: %d, Flags: %d", (int) processor_lapic->uid, (int) processor_lapic->apic_id, (int)processor_lapic->flags);
        } break;
        case 1:
        {
            printf(KERNEL_INFO, "IOAPIC_ID: %d, Addr: %x, GSIBase: %d ", (int)apic_table->info.ioapic.id, (int)apic_table->info.ioapic.address, (int)apic_table->info.ioapic.gsi_base);
            ioapic_allocate(apic_table->info.ioapic.id,
                            (u32)apic_table->info.ioapic.address,
                            (int)apic_table->info.ioapic.gsi_base);
        } break;
        case 2:
        {
            //printf(KERNEL_INFO, "Bus: %d, Src: %d, GSI: %d, Flags: %x, p: %d, t: %d, r: %d ", (int)apic_table->info.int_src_override.bus, (int)apic_table->info.int_src_override.source, (int)apic_table->info.int_src_override.gsi, (int)apic_table->info.int_src_override.flags, (int)apic_table->info.int_src_override.polarity, (int)apic_table->info.int_src_override.trigger, (int)apic_table->info.int_src_override.reserved);

            interrupt_set_override(apic_table->info.int_src_override.bus,
                                   apic_table->info.int_src_override.source,
                                   apic_table->info.int_src_override.gsi,
                                   apic_table->info.int_src_override.polarity,
                                   apic_table->info.int_src_override.trigger);

        } break;
        case 3:
        {
            printf(KERNEL_INFO, "Flags: %x GSI: %d ", (int)apic_table->info.nmi_source.flags, (int)apic_table->info.nmi_source.gsi);
        } break;
        case 4:
        {
            printf(KERNEL_INFO, "UID: %d, lint: %d, flags: %d ", (int)apic_table->info.lapic_nmi.uid, (int)apic_table->info.lapic_nmi.lapic_lint, (int)apic_table->info.lapic_nmi.flags);
        } break;

        }

        apic_table = (APIC_TABLE*)ADDPTRS(apic_table, apic_table->length);
    } while(remaining_bytes);
}

void acpi_process_table(ACPI_TABLE *acpi_table) {
    char sig[5];

    map_kernel_linear_with_pagetable((addr_t)acpi_table, acpi_table->length, PTE_PRESENT, false);
    for(int i=0;i<4;i++)
        sig[i] = acpi_table->signature[i];
    sig[4] = 0;
    printf(KERNEL_INFO, "ACPI Table: %p, %s, %d, %d ", acpi_table, sig, acpi_table->length, sizeof(ACPI_TABLE));

    if (!strncmp(acpi_table->signature, APIC_STR, strlen(APIC_STR))) {
        acpi_process_madt(acpi_table);
    } else if (!strncmp(acpi_table->signature, FACP_STR, strlen(FACP_STR))) {
        u16 iapc_boot_arch = *(u16*) ADDPTRS(acpi_table, 109);
        printf(KERNEL_INFO, "iapc_boot_arch: %d\n",(int)iapc_boot_arch);
        if (iapc_boot_arch & 2) {
            keyboard_controller_8042_present = true;
            printf(KERNEL_INFO, "8042 controller present\n");
        }
    }
}

void acpi_init() {

    rsdp = NULL;
    acpi_tables = NULL;

    if (!acpi_find_rsdp()) {
        printf(KERNEL_INFO, "No ACPI!!!\n");
        return;
    }

    rsdt = (ACPI_TABLE*)ADDPTRS(rsdp->rsdt_address, KERNEL_VIRT_ADDR);
    printf(KERNEL_INFO, "RSDT: %p ", (long)rsdt);
    map_kernel_linear_with_pagetable((addr_t)rsdt, sizeof(ACPI_TABLE), PTE_PRESENT, 0);
    map_kernel_linear_with_pagetable((addr_t)rsdt, rsdt->length, PTE_PRESENT, 0);
    printf(KERNEL_INFO, "RSDT: %p ", (long)rsdt-KERNEL_VIRT_ADDR);
    printf(KERNEL_INFO, "RSDT Length: %d, %d ", rsdt->length, sizeof(ACPI_TABLE));

    num_acpi_tables = (rsdt->length - sizeof(ACPI_TABLE)) / sizeof(void*);
    acpi_tables = (ACPI_TABLE**)ADDPTRS(rsdt, sizeof(ACPI_TABLE));

    for(int i=0;i<num_acpi_tables;i++) {
        acpi_process_table((ACPI_TABLE*)ADDPTRS(acpi_tables[i], KERNEL_VIRT_ADDR));
    }
    printf(KERNEL_INFO, "\n");
}
