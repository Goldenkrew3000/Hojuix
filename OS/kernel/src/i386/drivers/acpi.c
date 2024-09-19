#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <kernel/drivers/acpi.h>
#include <kernel_ext/limine.h>

/*
// ACPI Notes
// All addresses are PHYSICAL
// The only tables with 8 byte length signatures are the RSDP and XSDP
*/

#define HHDT_OFFSET 0xFFFF800000000000

// ACPI Variables
uint64_t* fadt_table_addr;
uint64_t* madt_table_addr;

// Request the ACPI RSDP Location
__attribute__((used, section(".requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

// RSDP / XSDP Structs
typedef struct RSDP_t {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_addr;
} __attribute__((packed));

typedef struct XSDP_t {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_addr;
    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed));
struct XSDP_t* xsdp;

// Generic ACPI Struct
typedef struct {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) ACPI_t;

// XSDT Struct (Contains Generic ACPI header and an array of pointers to other APCI tables)
typedef struct {
    ACPI_t acpi_header;
    uint64_t table_pointers[];
} __attribute__((packed)) XSDT_t;

// FADT Generic Address Structure Struct
typedef struct {
    uint8_t address_space;
    uint8_t bit_width;
    uint8_t bit_offset;
    uint8_t access_size;
    uint64_t address;
} __attribute__((packed)) GenericAddressStructure_t;

// FADT Struct
typedef struct {
    ACPI_t acpi_header;
    uint32_t firmware_ctrl;
    uint32_t dsdt;
    uint8_t reserved; // Field not used anymore, but kept for compatibility
    uint8_t preferred_power_management_profile;
    uint16_t sci_interrupt;
    uint32_t smi_command_port;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4bios_req;
    uint8_t pstate_control;
    uint32_t pm1a_event_block;
    uint32_t pm1b_event_block;
    uint32_t pm1a_control_block;
    uint32_t pm1b_control_block;
    uint32_t pm2_control_block;
    uint32_t pm_timer_block;
    uint32_t gpe0_block;
    uint32_t gpe1_block;
    uint8_t pm1_event_length;
    uint8_t pm1_control_length;
    uint8_t pm2_control_length;
    uint8_t pm_timer_length;
    uint8_t gpe0_length;
    uint8_t gpe1_length;
    uint8_t gpe1_base;
    uint8_t cstate_control;
    uint16_t worst_c2_latency;
    uint16_t worst_c3_latency;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t duty_offset;
    uint8_t duty_width;
    uint8_t day_alarm;
    uint8_t month_alarm;
    uint8_t century;
    uint16_t boot_architecture_flags;
    uint8_t reserved2;
    uint32_t flags;
    GenericAddressStructure_t reset_reg;
    uint8_t reset_value;
    uint8_t reserved3[3];
    uint64_t x_firmware_control;
    uint64_t x_dsdt;
    GenericAddressStructure_t x_pm1a_event_block;
    GenericAddressStructure_t x_pm1b_event_block;
    GenericAddressStructure_t x_pm1a_control_block;
    GenericAddressStructure_t x_pm1b_control_block;
    GenericAddressStructure_t x_pm2_control_block;
    GenericAddressStructure_t x_pm_timer_block;
    GenericAddressStructure_t x_gpe0_block;
    GenericAddressStructure_t x_gpe1_block;
} __attribute__((packed)) FADT_t;

void acpi_init() {
    printf("Initializing ACPI...\n");

    // Get the RSDP Pointer from Limine and load the RSDP table
    uint64_t* rsdp_addr = rsdp_request.response->address;
    struct RSDP_t* rsdp = (struct RSDP_t*)rsdp_addr;

    // TODO: Check the RSDP Checksum
    
    // Check RSDP for the ACPI version
    if (rsdp->revision == 0) {
        printf("Your system is using ACPI 1.0, which is not supported.\n");
        abort();
    } else if (rsdp->revision == 2) {
        printf("ACPI 2.0 - 6.1 detected.\n");
        // In ACPI 2.0+, the XSDP is used instead of the RSDP
        xsdp = (struct XSDP_t*)rsdp_addr;

        // Jump to the XSDT handler
        acpi_handle_xsdt();
    }
}

void acpi_handle_xsdt() {
    // Load the XSDT pointer from the XSDP into the XSDT Struct
    uint64_t* xsdt_addr = (uint64_t*)((uintptr_t)xsdp->xsdt_addr + HHDT_OFFSET);
    XSDT_t* xsdt = (XSDT_t*)xsdt_addr;

    // Check the checksum of the XSDT table
    //unsigned char xsdt_checksum_sum = 0;
    //for (int i = 0; i < xsdt->acpi_header.length; i++) {
    //    printf("spam\n");
    //    xsdt_checksum_sum += ((char*)xsdt->acpi_header[i]);
    //}
    //printf("SUM: %d\n", xsdt_checksum_sum);

    // Load pointers for the other tables
    uint64_t* table_addrs = (uint64_t*)xsdt->table_pointers;
    int table_entries = (xsdt->acpi_header.length - sizeof(ACPI_t)) / sizeof(uint64_t);
    printf("XSDT: Entry pointers: %d\n", table_entries);

    // Check the tables at the pointers for desired ACPI tables
    for (int i = 0; i < table_entries; i++) {
        uint64_t* addr = (uint64_t*)((uintptr_t)table_addrs[i] + HHDT_OFFSET);
        ACPI_t* table = (ACPI_t*)addr;
        
        // Search for certain ACPI tables
        // NOTE: The strings are NOT null terminated, hence the use of memcmp
        if (memcmp(table->signature, "FACP", 4) == 0) {
            // Found the FACP / FADT Table
            fadt_table_addr = (uint64_t*)addr;
        }

        if (memcmp(table->signature, "APIC", 4) == 0) {
            // Found the APIC / MADT Table
            madt_table_addr = (uint64_t*)addr;
        }
    }

    // Jump to the FADT handler
    acpi_handle_fadt();
}

#include <kernel/io.h>
#include <kernel/timer.h>

void acpi_handle_fadt() {
    // Load the FADT pointer into the FADT struct
    uint64_t* fadt_addr = (uint64_t*)fadt_table_addr; // NOTE: Offset already applied from above
    FADT_t* fadt = (FADT_t*)fadt_addr;

    //printf("FADT sig: %p\n", fadt->reset_reg.address);
    //printf("Reser val: %.2x\n", fadt->reset_value);
    //uint64_t* reset_reg_final = fadt->reset_reg.address + (uint64_t)hhdt_offset;
    //printf("final addr: %p\n", reset_reg_final);

    //printf("ACPI rebooting in 5 seconds...");
    //timer_wait(5000);
    //outb(reset_reg_final, fadt->reset_value);
}
