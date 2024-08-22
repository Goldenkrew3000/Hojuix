#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h> // Note to self: DO NOT REMOVE STRING.H, Clang can call memcpy, memset, memmov, and memcmp automatically, hence they are needed
#include <kernel/dt.h>
#include <kernel/io.h>
#include <kernel/timer.h>
#include <kernel_ext/limine.h>
#include <kernel/framebuffer.h>

// testing
#include <kernel/dt.h>

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

// ---
__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

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
    
    // Enable interrupts
    asm volatile("sti");

    pit_timer_install();

    // Print display info to console
    printf("Framebuffer Size: %dx", framebuffer->width);
    printf("%d\n", framebuffer->height);

    // TEST Scrolling
    //framebuffer_scroll(1); // 4 lines per character
    

    // TEST Memmap
    if (memmap_request.response == NULL) {
        krnl_halt();
    }

    uint64_t total_usable_mem = 0;
    uint64_t total_reserved_mem = 0;

    uint64_t entry_count = memmap_request.response->entry_count;
    struct limine_memmap_entry **entries = memmap_request.response->entries;
     for (uint64_t i = 0; i < entry_count; i++) {
        struct limine_memmap_entry *entry = entries[i];

        printf("Mem Region %llu: ", i);
        printf("Base: 0x%lx, ", entry->base);
        printf("Length: 0x%lx, ", entry->length);
        printf("Type: %u\n", entry->type);

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            total_usable_mem += entry->length;
        } else {
            total_reserved_mem += entry->length;
        }
    }

    printf("Total reserved mem: %lld kb.\n", total_reserved_mem / 1024);
    printf("Total usable mem: %lld kb.\n", total_usable_mem / 1024);

    // Enable Keyboard
    

    //uint64_t fb_width = fb->width;


    // NOTE: Assume the framebuffer model is RGB with 32bit pixels
    
    //kernel_draw_pixel(framebuffer, 0, 0, 0xFF0000);
    

    
    //framebuffer_putstring("Hello World!");

    
    printf("Setup!\n");

    ///int num2 = 20 / 0;
    //printf("%d\n", num2);
    
    

    while(1) { }

    //hcf();
    //krnl_halt();
}
