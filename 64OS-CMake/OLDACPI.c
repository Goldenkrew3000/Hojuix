#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <kernel/acpi.h>
#include <kernel_ext/limine.h>

/*
 * ACPI Notes
 * The 8 byte signature on the RSDP and XSDP are an exception,
 * all other ACPI tables have a 4 byte header
 */

// Request the ACPI RSDP Location
__attribute__((used, section(".requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

// Structs to hold RSDP information
// RSDP_t is used for ACPI 1.0
typedef struct RSDP_t {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_addr; // NOTE: 32 bit physical addr
} __attribute__((packed));

// XSDP_t is used for ACPI 2.0+
typedef struct XSDP_t {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_addr; // Deprecated since ACPI 2.0
    uint32_t length;
    uint64_t xsdt_addr; // NOTE: 64 bit physical addr
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed));
struct XSDP_t* xsdp_info;

// Struct to hold the XSDT (Extended System Descriptor Table)
typedef struct {
    char signature[4];
    uint32_t length; // Total size of the table, including the header
    uint8_t revision;
    uint8_t checksum; // Example here https://wiki.osdev.org/
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute((packed)) ACPI_t;

typedef struct {
    ACPI_t header;
    uint64_t tables[];
} __attribute__((packed)) XSDT;


typedef struct {
    ACPI_t header;
} __attribute__((packed)) FACP;

typedef struct FACP_t {
    // So it looks like the FACP/FADT contains a general ACPI header
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
    // And then the specific stuff
    uint32_t firmware_ctrl;
    uint32_t dsdt;
    uint8_t reserved; // Field only used in ACPI 1.0, only here for compatibility
    uint8_t preferred_power_management_profile;
    uint16_t sci_interrupt;
    uint32_t smi_command_port;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4bios_req;
    uint8_t pstate_control;
} __attribute__((packed));
struct FACP_t* facp_table;

// Functions to move to header file
void acpi_rsdp_handler();
void acpi_xsdt_handler();
void acpi_facp_handler();

void acpi_init() {
    printf("Initializing ACPI...\n");

    acpi_rsdp_handler();
}

void acpi_rsdp_handler() {
    // Get the RSDP pointer, and put the data into the RSDP struct
    uint64_t* rsdp_addr = rsdp_request.response->address;
    struct RSDP_t* rsdp_info = (struct RSDP_t*)rsdp_addr;

    // TODO: Check the checksum of the ACPI RDSP table: https://wiki.osdev.org/RSDP#Detecting_ACPI_Version

    // Check RSDP version
    if (rsdp_info->revision == 0) {
        printf("ACPI 1.0 detected.\n");
        // TODO: ACPI 1.0 support
    } else if (rsdp_info->revision == 2) {
        printf("ACPI 2.0 - 6.1 detected.\n");
        // In ACPI 2.0+, XSDP is used instead of RSDP, so load data into the XSDP struct instead
        // TODO: Check if signature contains "RSD PTR " (yes, including the end space)
        xsdp_info = (struct XSDP_t*)rsdp_addr;
        printf("ACPI - OEM ID: %.6s\n", xsdp_info->oem_id);
        printf("XSDT Addr: %p\n", xsdp_info->xsdt_addr);

        // Call the XSDT handler
        acpi_xsdt_handler();
        
        // Call the FACP handler
        acpi_facp_handler();
    }
}

uint64_t* facp_ptr;
uint64_t* apic_ptr;

void acpi_xsdt_handler() {
    // Load the XSDT data into the XSDT struct from the XDST pointer stored in the XSDP struct
    // But first, add the HHDM offset to the XSDT Physical Address
    // TODO: Grab the HHDM offset from limine, not hardcoded
    uint64_t* xsdt_addr = xsdp_info->xsdt_addr + 0xFFFF800000000000;
    XSDT* xsdt = (XSDT*)xsdt_addr;
    printf("XSDT Address: %p\n", xsdt_addr);
    printf("XSDT Signature: %.4s\n", xsdt->header.signature);
    printf("XSDT OEMID: %.6s\n", xsdt->header.oem_id);
    printf("XSDT OEMTableID: %.8s\n", xsdt->header.oem_table_id);

    // Grab pointers for the other tables
    uint64_t* entry_ptrs = (uint64_t*)xsdt->tables;
    int entries = (xsdt->header.length - sizeof(ACPI_t)) / sizeof(uint64_t);
    printf("Entries: %d\n", entries);

    // Search tables for a specific table
    // FACP --> FADT Table
    // APIC --> MADT Table
    for (int i = 0; i < entries; i++) {
        uint64_t* ptr = 0xFFFF800000000000 + entry_ptrs[i];
        ACPI_t* table = (ACPI_t*)ptr;
        printf("Table: %.4s\n", table->signature);

        // These strings are NOT null terminated, hence the use of memcmp

        if (memcmp(table->signature, "FACP", 4) == 0) {
            printf("Found FADT Table!\n");
            facp_ptr = (uint64_t)entry_ptrs[i];
        }

        if (memcmp(table->signature, "APIC", 4) == 0) {
            printf("Found MADT Table!\n");
            apic_ptr = (uint64_t)entry_ptrs[i];
        }
    }
}

void acpi_facp_handler() {
    uint64_t* facp_ptr_hhdt = 0xFFFF800000000000 + (uint64_t)facp_ptr;
    printf("FACP Pointer: %p\n", facp_ptr_hhdt);
    facp_table = (struct FACP_t*)facp_ptr_hhdt;
    printf("FACP Signature: %.4s\n", facp_table->signature);

    printf("FACP Pref Powr Mgmt Prof: %.2x\n", facp_table->preferred_power_management_profile);
}
