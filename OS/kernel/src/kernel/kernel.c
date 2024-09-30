#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
// NOTE: String.h is needed so clang can call [ memcpy, memset, memmov, memcmp ] independently
#include <string.h>
#include <kernel.h>

#include <kernel/i386/gdt.h>

#include <kernel/i386/dt.h>
#include <kernel/i386/io.h>
#include <kernel/drivers/pit_timer.h>
#include <kernel/drivers/framebuffer.h>
#include <kernel/drivers/acpi.h>
#include <kernel/drivers/pci.h>
#include <kernel/memory/pmmgr.h>
#include <kernel/memory/vmmgr.h>
#include <kernel_ext/limine.h>
#include <kernel/drivers/rs232.h>

// Set the limine base revision to 2 (Latest)
__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

// Request the Framebuffer
__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Request the Memory Map
__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

// Request the HHDM Offset
__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

// Request the Kernel Address
volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

// Request the ACPI RSDP Address
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

// Request the SMP Info
static volatile struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0
};

// Define the start and end markers for the limine requests
__attribute__((used, section(".request_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

// Create Global Kernel Data Storage
kernel_t kerndata = {0};

void kernel_entry(void) {
    // Disable interrupts
    asm volatile("cli");

    // Ensure limine understands our base revision
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        asm volatile("hlt");
    }

    // Ensure we have got a framebuffer
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        // Could not fetch a framebuffer from limine
        asm volatile("hlt");
    }

    // Fetch the first framebuffer
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    // Push required data to global kernel data storage
    kerndata.hhdm_offset = (hhdm_request.response)->offset;
    kerndata.memmap = *memmap_request.response;
    kerndata.kernel_addr = *(kernel_address_request.response);
    kerndata.smp_response = smp_request.response;

    // Init framebuffer
    framebuffer_ssfn_init(framebuffer);
    framebuffer_fill_background(0x000000);

    // Print compile time to console
    printf("HOJUIX 0.3A x86_64 - Build Time: %s ", __DATE__);
    printf("%s\n", __TIME__);

    // Print CPUID
    //print_cpuid();

    // Initialize Memory Managers
    //pmmgr_init();
    //vmmgr_init();
    //vmmgr_switch_structures();

    // Initialize GDT
    printf("Initializing GDT...");
    //gdt_init();
    gdt_init();
    printf(" OK\n");

    // Initialize IDT
    printf("Initializing IDT...");
    idt_init();
    printf(" OK\n");

    // Initialize IRQ
    printf("Initializing IRQ...");
    irq_init();
    printf(" OK\n");
     
    // Print display info to console
    printf("Framebuffer Size: %dx", framebuffer->width);
    printf("%d\n", framebuffer->height);

    // Initialze PMM
    //pmmgr_init();
    //vmmgr_init();
    printf("Kern CR3: %p\n", kerndata.cr3);
    //printf("Switching\n");
    //switch_page_structures();
    //kernheap_init();
    //pmmgr_print_bitmap();

    // Initialize COM1 @ 115200bps
    //rs232_init(1, 115200);

    // Initialize VMM
    //vmmgr_init(); // Still completely non functional
    //pmmgr_print_bitmap();

    // Enable interrupts
    asm volatile("sti");

    // Initialize RS232
    //int ret = rs232_init(1, 57600);
    //rs232_writeline(1, "Hello world from RS232!\n");


    // Initialize ACPI
    acpi_init();

    // Initialize PCI
    pci_init();

    // WORKING - Attempt to retrieve a pci device from the pci device table
    //struct t_pci_device *pci_device_a = (struct t_pci_device*)(kerndata.pci_devices_addr + (uint64_t)(0x9 * 1));
    //printf("Device Vendor: %x\n", pci_device_a->device_id);

    // Halt kernel, but in a running state
    printf("[KERNEL] Reached end of kernel.\n");
    while(1) { }
    asm volatile("cli; hlt");
}
