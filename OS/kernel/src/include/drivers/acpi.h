#ifndef _ACPI_H
#define _ACPI_H

/*
// RSDP / XSDP
*/
typedef struct {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_addr;
} __attribute__((packed)) RSDP_t;

typedef struct {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_addr;
    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed)) XSDP_t;

/*
// XSDT
*/
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

// XSDT Struct
typedef struct {
    ACPI_t acpi_header;
    uint64_t table_pointers[]; // Undefined length, could be 5 tables, could be 30
} __attribute__((packed)) XSDT_t;

/*
// FADT
*/
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

/*
// MADT
*/
#define ACPI_MADT_ENTRY_TYPE_0_LENGTH 8     // Processor Local APIC
#define ACPI_MADT_ENTRY_TYPE_1_LENGTH 12    // I/O APIC
#define ACPI_MADT_ENTRY_TYPE_2_LENGTH 10    // I/O APIC Interrupt Source Override
#define ACPI_MADT_ENTRY_TYPE_3_LENGTH 10    // I/O APIC Non-maskable interrupt source
#define ACPI_MADT_ENTRY_TYPE_4_LENGTH 6     // Local APIC Non-maskable interrupts
#define ACPI_MADT_ENTRY_TYPE_5_LENGTH 12    // Local APIC Address Override
#define ACPI_MADT_ENTRY_TYPE_9_LENGTH 16    // Processor Local x2APIC

// MADT Struct
typedef struct {
    ACPI_t acpi_header;
    uint32_t local_apic_addr;
    uint32_t local_apic_flags;
} __attribute__((packed)) MADT_t;

// MADT Entry Start
typedef struct {
    uint8_t type;           // Entry Type
    uint8_t length;         // Entry Length
} __attribute__((packed)) MADT_Entry_t;

// MADT Entry Type 0 Struct
typedef struct {
    MADT_Entry_t entry_hdr;
    uint8_t acpi_proc_id;   // ACPI Processor ID
    uint8_t apic_id;        // APIC ID
    uint32_t flags;         // Flags
} __attribute__((packed)) MADT_Type0_t;

// MADT Entry Type 1 Struct
typedef struct {
    MADT_Entry_t entry_hdr;
    uint8_t io_apic_id;     // I/O APIC's ID
    uint8_t rsvd;
    uint32_t io_apic_addr;  // I/O APIC Address
    uint32_t gsib;          // Global System Interrupt Base
} __attribute__((packed)) MADT_Type1_t;

// MADT Entry Type 2 Struct
typedef struct {
    MADT_Entry_t entry_hdr;
    uint8_t bus_source;     // Bus Source
    uint8_t irq_source;     // IRQ Source
    uint32_t gsi;           // Global System Interrupt
    uint16_t flags;         // Flags
} __attribute__((packed)) MADT_Type2_t;

// MADT Entry Type 3 Struct
typedef struct {
    MADT_Entry_t entry_hdr;
    uint8_t nmi_source;     // NMI Source
    uint8_t rsvd;
    uint16_t flags;         // Flags
    uint32_t gsi;           // Global System Interrupt
} __attribute__((packed)) MADT_Type3_t;

// MADT Entry Type 4 Struct
typedef struct {
    MADT_Entry_t entry_hdr;
    uint8_t acpi_proc_id;   // ACPI Processor ID (0xff means all)
    uint16_t flags;         // Flags
    uint8_t lint;           // LINT# (0 or 1)
} __attribute__((packed)) MADT_Type4_t;

// MADT Entry Type 5 Struct
typedef struct {
    MADT_Entry_t entry_hdr;
    uint16_t rsvd;
    uint64_t apic_addr;     // 64-bit physical address of the Local APIC
} __attribute__((packed)) MADT_Type5_t;

// MADT Entry Type 9 Struct
typedef struct {
    MADT_Entry_t entry_hdr;
    uint16_t rsvd;
    uint32_t x2apic_id;     // Processor's local x2APIC ID
    uint32_t flags;         // Flags
    uint32_t acpi_id;       // ACPI ID
} __attribute__((packed)) MADT_Type9_t;

void acpi_init();
void acpi_handle_xsdt(uintptr_t xsdt_addr);
void acpi_handle_fadt(uintptr_t fadt_addr);
void acpi_handle_madt(uintptr_t madt_addr);
void acpi_shutdown();

#endif
