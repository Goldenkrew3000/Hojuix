#ifndef _KERNEL_H
#define _KERNEL_H
#include <kernel_ext/limine.h>
#include <kernel/i386/idt.h>
#include <kernel/drivers/pci.h>

typedef struct {
    // Info from Limine
    struct limine_memmap_response memmap;
    struct limine_kernel_address_response kernel_addr;
    struct limine_framebuffer **framebuffer;
    struct limine_smp_response *smp_response;
    uint64_t hhdm_offset;

    // GDT / IDT / TSS
    struct idtr_t idtr;

    // ACPI Addresses

    // PCI
    uint64_t pci_devices_addr;
    int pci_device_count;
    
    // Memory Management (PMM / VMM) Info
    long last_freed_page;
    long last_freed_section;
    uint64_t last_freed_num_pages;
    uintptr_t kernheap_start;
    uint64_t cr3; // Physical PML4 CR3
   
    // Debugging
    int debug_port;
} kernel_t;
extern kernel_t kerndata;

#endif
