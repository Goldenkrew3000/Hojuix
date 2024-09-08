#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h> // Note to self: DO NOT REMOVE STRING.H, Clang can call memcpy, memset, memmov, and memcmp automatically, hence they are needed
#include <kernel/dt.h>
#include <kernel/io.h>
#include <kernel/timer.h>
#include <kernel/framebuffer.h>
#include <kernel/utils.h>
#include <kernel/dt.h>
#include <kernel/drivers/acpi.h>
#include <kernel/drivers/pci.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <kernel/shell.h>
#include <kernel_ext/limine.h>

#include <kernel/drivers/rs232.h>

extern void krnl_halt();

// Set the limine base revision to 2 (Latest)
__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

// Setup the limine framebuffer
__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Request the HHDM Offset
/*__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};*/

// Define the start and end markers for the limine requests
__attribute__((used, section(".request_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

void kernel_entry(void) {
    // KERNEL ENTRY POINT

    // Disable interrupts (Not done by limine)
    asm volatile("cli");

    // Ensure limine understands our base revision
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        krnl_halt();
    }

    // Ensure we have got a framebuffer
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        krnl_halt();
    }

    // Fetch the first framebuffer
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    // Init framebuffer
    framebuffer_ssfn_init(framebuffer);
    framebuffer_fill_background(0x3F4452);

    // Print compile time to console
    printf("HOJUIX 0.2A x86_64 - Build Time: %s ", __DATE__);
    printf("%s\n", __TIME__);

    // Print CPUID
    print_cpuid();

    // Initialize GDT
    printf("Initializing GDT...");
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

    // Initialize PMM
    pmmgr_init();
    pmmgr_print_bitmap();

    // Initialize VMM
    //vmmgr_init();
    //pmmgr_print_bitmap();

    // Enable interrupts
    asm volatile("sti");

    // Initialize RS232
    int ret = rs232_init(1, 57600);
    rs232_writeline(1, "Hello world from RS232!\n");


    // Initialize ACPI
    acpi_init();

    // Initialize PCI
    //pci_init();

    // Drop into kernel mode shell
    shell_init();
    
    // Halt kernel, but in a running state
    while(1) { }
    krnl_halt();
}
