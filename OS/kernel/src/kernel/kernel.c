#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
// NOTE: String.h is needed so clang can call [ memcpy, memset, memmov, memcmp ] independently
#include <string.h>
#include <kernel.h>
#include <i386/gdt.h>
#include <i386/idt.h>
#include <i386/irq.h>
#include <i386/cpuinfo.h>
#include <drivers/ps2_keyboard.h>
#include <i386/io.h>
#include <drivers/i386/pit_timer.h>
#include <drivers/framebuffer.h>
#include <drivers/acpi.h>
#include <drivers/pci.h>
#include <memory/pmmgr.h>
#include <memory/vmmgr.h>
#include <drivers/rs232.h>
#include <drivers/ata_pio.h>
#include <fs/fat16.h>
#include <process/elf.h>
#include <i386/asm_functions.h>
#include <kernel_ext/limine.h>
#include <drivers/ahci.h>

#include <i386/spinlock.h>
#include <stdatomic.h>

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

    // Fetch HHDM offset, Memory map, and Kernel address to configure memory management
    //rs232_writeline(1, "[LIMINE] Fetching HHDM Offset, Memory map, and Kernel address.\r\n");
    kerndata.hhdm_offset = (hhdm_request.response)->offset;
    kerndata.memmap = *memmap_request.response;
    kerndata.kernel_addr = *(kernel_address_request.response);

    // Initialize the Memory Managers
    pmmgr_init();
    vmmgr_init();
    vmmgr_switch_cr3();
    i386_memset((uint8_t*)KERNEL_STACK_ADDR, 0x00, 4096 * 16); // Zero stack now that it is mapped
    vmmgr_switch_stack(); // Switch to new stack at 0xfffffffffffff000

    // Ensure we have got a framebuffer
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        // Could not fetch a framebuffer from limine
        rs232_writeline(1, "[LIMINE] Could not find a framebuffer, halting.\r\n");
        asm volatile("hlt");
    } else {
        rs232_writeline(1, "[LIMINE] Found a framebuffer.\r\n");
    }

    // Fetch the first framebuffer
    struct limine_framebuffer *framebuffer = NULL;
    framebuffer = framebuffer_request.response->framebuffers[0];

    // Init framebuffer
    framebuffer_ssfn_init(framebuffer);
    framebuffer_fill_background(0x000000);

    // Make first prints to the framebuffer
    printf("HOJUIX 0.4A (KMode Paging) x86_64 - Build Time: %s %s\n", __DATE__, __TIME__);
    pmmgr_print_bitmap();
    print_cpuid();

    // Initialize GDT
    tss_init();
    gdt_init();

    // Initialize IDT
    idt_init();

    // Initialize IRQ
    irq_init();

    pmmgr_print_bitmap();

    // Print display info to console
    //printf("Framebuffer Size: %dx", framebuffer->width);
    //printf("%d\n", framebuffer->height);

    // Initialize COM1 @ 115200bps
    //rs232_init(1, 115200);

    // Enable interrupts
    asm volatile("sti");

    // Initialize the PIT Timer
    //pit_timer_init();

    // Initialize PS2 Keyboard
    //ps2_keyboard_init();



    // Initialize ACPI
    acpi_init();

    // Initialize PCI
    //ata_pio_init();
    //fat16_fs_test();

    pci_init();
    int ahci_idx = pci_find_ahci_device();
    uintptr_t bar_tbl_addr = pci_fetch_bar(ahci_idx);
    pci_device_bar_table_t* bar_tbl = (pci_device_bar_table_t*)bar_tbl_addr;
    printf("BAR4 Exists: %d\n", bar_tbl->bar4_exists);
    printf("BAR5 Exists: %d\n", bar_tbl->bar5_exists);
    printf("BAR4 MMIO: %d\n", bar_tbl->bar4_mmio);
    printf("BAR5 MMIO: %d\n", bar_tbl->bar5_mmio);
    printf("BAR5 Address: 0x%lx\n", bar_tbl->bar5_addr);
    printf("BAR5 Size: %ld bytes\n", bar_tbl->bar5_bar_size);


    ahci_init();

    //nvme_init();
    //dump_first_sector();
    //ac97_init();
    //generate_tone(500, 50000);
    //play_large_buffer();
    //for (volatile int i = 0; i < 1000000; i++);
    

    // WORKING - Attempt to retrieve a pci device from the pci device table
    //struct t_pci_device *pci_device_a = (struct t_pci_device*)(kerndata.pci_devices_addr + (uint64_t)(0x9 * 1));
    //printf("Device Vendor: %x\n", pci_device_a->device_id);

    /*
    printf("SPLK test start\n");
    spinlock_t sl = SPINLOCK_INIT;
    spinlock_acquire(&sl.lock);
    printf("SPLK test end\n");
    spinlock_release(&sl.lock);
    */

    //ata_pio_init();
    //fat16_fs_test();
    //vmmgr_map_usermode(); // Prepare the stack and the exec page
    //run_usermode();

    // USERSPACE!!

    /*    asm volatile("cli");
    printf("Boo! Surprise usermode test!!!! Hope u studied :3\n");
    vmmgr_map_usermode();
    printf("hmmm");
    lala();
    //asm volatile("call *%0\n" : )
    //uint8_t* function
    printf("GHAHAHA\n");
    kernel_test(0xffff800001693000);
    printf("Attempting to execute usermode code...\n");
    usermode(0x6000000000, 0x700000000000);

    printf("FAIL");
    user_entry();
    */

    //vmmgr_map_usermode();
    //void (*user_code)() = (void(*)())0x400000;
    //uint64_t user_stack = 0x700000000000;             // User stack location
    //uint64_t if_addr=0x200;

    //__asm__ volatile (
    //    "mov %0, %%r14\n\t"   // User code address in R14
    //    "mov %1, %%r15\n\t"   // User stack in R15
    //    "call usermode2"
    //    :: "r"(user_code), "r"(user_stack)
    //    : "r14", "r15", "memory"
    //);
    //printf("Returned from userspace!!\n");

    // Halt kernel, but in a running state
    kernel_finished();
}

void kernel_finished() {
    printf("[KERNEL] Reached end of kernel.\n");
    while(1) { asm volatile("nop"); }
    asm volatile("cli; hlt");
}

void run_usermode() {
    elf_prepare(0x9010000000); // Hardcoded address from the fat16 driver
    void (*user_code)() = (void(*)())0x400000;
    uint64_t user_stack = 0x700000000000;
    uint64_t if_addr=0x200;

    printf("\n");
    __asm__ volatile (
        "mov %0, %%r14\n\t"   // User code address in R14
        "mov %1, %%r15\n\t"   // User stack in R15
        "call launch_usermode"
        :: "r"(user_code), "r"(user_stack)
        : "r14", "r15", "memory"
    );
}
