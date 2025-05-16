#ifndef _KERNEL_H
#define _KERNEL_H
#include <i386/idt.h>
#include <drivers/pci.h>
#include <kernel_ext/limine.h>
#include <errno.h>

// Types (XV6 stat.h)
#define T_DIR    0x1
#define T_FILE   0x2
#define T_DEVICE 0x3

// File Options (fcntl)
#define O_RDONLY 0x0
#define O_WRONLY 0x1
#define O_RDWR   0x2
#define O_CREATE 0x3

#define EXIT_SUCCESS 0x0
#define EXIT_FAILURE 0x1

void run_usermode();
void kernel_finished();

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
