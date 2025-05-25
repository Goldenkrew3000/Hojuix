/*
// Hojuix ACPI Driver
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <kern/kprintf.h>
#include <kern/libkern.h>
#include <drivers/acpi.h>
#include <arch/amd64/io.h>
#include <memory/vmmgr.h>
#include <kernel_ext/limine.h>

/*
// ACPI Notes
// All addresses are PHYSICAL
// The only tables with 8 byte length signatures are the RSDP and XSDP
*/

uintptr_t hhdt_offset = 0xFFFF800000000000;

// Request the ACPI RSDP Location
__attribute__((used, section(".requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

void acpi_init() {
    printf("Initializing ACPI...\n");

    // Get the RSDP Pointer from Limine and load the RSDP table
    uintptr_t rsdp_addr = (uintptr_t)rsdp_request.response->address;
    RSDP_t* rsdp = (RSDP_t*)rsdp_addr;

    // TODO: Check the RSDP Checksum

    // Check RSDP for the ACPI version
    if (rsdp->revision == 0) {
        printf("Your system is using ACPI 1.0, which is not supported.\n");
        // TODO Abort here
    } else if (rsdp->revision == 2) {
        printf("ACPI 2.0 - 6.1 detected.\n");
        // In ACPI 2.0+, the XSDP is used instead of the RSDP
        XSDP_t* xsdp = (XSDP_t*)rsdp_addr;

        // Jump to the XSDT handler
        acpi_handle_xsdt(xsdp->xsdt_addr + hhdt_offset);
    }
}

void acpi_handle_xsdt(uintptr_t xsdt_addr) {
    XSDT_t* xsdt = (XSDT_t*)xsdt_addr;

    // Check the checksum of the XSDT table TODO

    // Load pointers for the other tables
    uintptr_t table_array_addr = (uintptr_t)xsdt->table_pointers;
    uint64_t* table_array = (uint64_t*)table_array_addr;
    int table_entries = (xsdt->acpi_header.length - sizeof(ACPI_t)) / sizeof(uint64_t);
    printf("XSDT: Entry pointers: %d\n", table_entries);

    // List all the tables
    /*
    for (int i = 0; i < table_entries; i++) {
        ACPI_t* table = (ACPI_t*)(table_array[i] + hhdt_offset);
        printf("New Table: %.4s\n", table->signature);
    }
        */

    // ACPI Variables
    //uintptr_t fadt_table_addr = 0;
    //uintptr_t madt_table_addr = 0;
    //uintptr_t ssdt_table_addr = 0;

    // Check the tables at the pointers for desired ACPI tables
    // NOTE: On real hardware, there are multiple tables called the same, so process them as we find them
    for (int i = 0; i < table_entries; i++) {
        ACPI_t* table = (ACPI_t*)(table_array[i] + hhdt_offset);

        // Search for ACPI tables
        // NOTE: The strings are NOT null terminated
        if (memcmp(table->signature, "FACP", 4) == 0) {
            // Found the FACP / FADT Table
            //printf("FADT addr: %p\n", table_array[i]);
            acpi_handle_fadt((uintptr_t)table_array[i] + hhdt_offset); // FADT is for Power Management (https://wiki.osdev.org/FADT)
        }

        if (memcmp(table->signature, "APIC", 4) == 0) {
            // Found the APIC / MADT Table
            //acpi_handle_madt((uintptr_t)table_array[i] + hhdt_offset); // MADT is for Clocks / Interrupts (https://wiki.osdev.org/MADT)
            //printf("MADT Addr: %p\n", table_array[i]);
        }

        if (memcmp(table->signature, "SSDT", 4) == 0) {
            // Found the SSDT Table
            //ssdt_table_addr = addr; // SSDT is a suppliment to DSDT (https://wiki.osdev.org/SSDT)
        }
    }

    /*
    // Jump to the FADT handler
    if (fadt_table_addr != NULL) {
        printf("Found FADT table address.\n");
        acpi_handle_fadt();
    }
        */
}

uint8_t acpi_reboot_val;
uintptr_t acpi_reboot_reg;
void acpi_handle_fadt(uintptr_t fadt_addr) {
    // Load the FADT pointer into the FADT struct
    FADT_t* fadt = (FADT_t*)fadt_addr;
    //uint64_t* fadt_addr = (uint64_t*)fadt_table_addr; // NOTE: Offset already applied from above
    //fadt = (FADT_t*)fadt_table_addr;

    
    //printf("FADT sig: %p\n", fadt->reset_reg.address);
    //printf("Reser val: %.2x\n", fadt->reset_value);
    acpi_reboot_val = fadt->reset_value;
    uintptr_t reset_reg_final = fadt->reset_reg.address + hhdt_offset;
    acpi_reboot_reg = reset_reg_final;
    //printf("final addr: %p\n", reset_reg_final);

    //printf("ACPI rebooting in 5 seconds...");
    //timer_wait(5000);
    //for (size_t i = 0; i < 1000000000; i++) { asm volatile("nop"); }
    //out8(reset_reg_final, fadt->reset_value);
}

void acpi_reboot() {
    out8(acpi_reboot_reg, acpi_reboot_val);
}

void acpi_handle_madt(uintptr_t madt_addr) {
    MADT_t* madt = (MADT_t*)madt_addr;
    uintptr_t madt_max_addr = madt_addr + (uintptr_t)madt->acpi_header.length;


    //printf("Local APIC Address: 0x%lx\n", madt->local_apic_addr);
    //printf("Local APIC Flags: %lx\n", madt->local_apic_flags);
    //printf("Length: %d\n", madt->acpi_header.length);

    // Calculate the address of the start of the entries
    uintptr_t madt_entries_start_addr = madt_addr + sizeof(MADT_t);
    //uint8_t* entry = (uint8_t*)madt_entries_start_addr;
    //printf("Entry type: %x\n", entry[0]);
    //printf("Entry length: %d\n", entry[1]);

    int running = 1;
    while (running == 1) {
        // Make array of memory location of entry start
        uint8_t* entry = (uint8_t*)madt_entries_start_addr;
        

        if ((uintptr_t)entry >= madt_max_addr) { running = 0; }

        // Calculate the address of the next entry
        madt_entries_start_addr += entry[1];

        /*
        switch (entry[0]) {
            case 0:
                printf("APIC Entry Type 0:\n");
                break;
            case 1:
                printf("APIC Entry Type 1:\n");
                break;
            case 2:
                printf("APIC Entry Type 2:\n");
                break;
            case 3:
                printf("APIC Entry Type 3:\n");
                break;
            case 4:
                printf("APIC Entry Type 4:\n");
                break;
            case 5:
                printf("APIC Entry Type 5:\n");
                break;
            case 9:
                printf("APIC Entry Type 9:\n");
                break;
            default:
                printf("Unknown APIC Entry\n");
        };*/

        
    }
    

    // TODO If bit 1 is set in the flags field, mask ALL PIC interrupts (but do it anyway)
    
    //printf("MADT Offset: 0x%lx\n", madt_off);
}

void acpi_shutdown() {  }
/*void acpi_shutdown() { // TODO Not working...
    // Perform ACPI shutdown (Issued from SYS_SHUTDOWN)
    printf("[ACPI] Performing ACPI Shutdown...\n");
    uintptr_t reset_reg_final = (uintptr_t)fadt->reset_reg.address + hhdt_offset;
    out8(reset_reg_final, fadt->reset_value);
    asm volatile("cli; hlt;");
}*/
