/*
// Hojuix x86_64 Kernel Entry
// 2025-05-01
// GPLv3
*/

#include <stdatomic.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <kernel.h>
#include <kern/kprintf.h>
#include <drivers/ps2_keyboard.h>
#include <drivers/i386/pit_timer.h>
#include <drivers/framebuffer.h>
#include <drivers/acpi.h>
#include <drivers/pci.h>
#include <drivers/rs232.h>
#include <memory/pmmgr.h>
#include <memory/vmmgr.h>
#include <fs/fat16.h>
#include <process/elf.h>
#include <drivers/ahci.h>
#include <drivers/nvme.h>
#include <drivers/xhci.h>
#include <fs/guid_pt.h>
#include <drivers/sound/hojuix_hda.h>
#include <fs/ext2.h>
#include <arch/amd64/spinlock.h>
#include <arch/amd64/gdt.h>
#include <arch/amd64/idt.h>
#include <arch/amd64/irq.h>
#include <arch/amd64/cpuinfo.h>
#include <arch/amd64/asm_functions.h>
#include <arch/amd64/io.h>
#include <kernel_ext/limine.h>

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

    // Initialize COM1 @ 115200bps
    //rs232_init(1, 115200);

    // Enable interrupts
    // NOTE: IRQ 0 and 1 are unmasked, others are masked, cannot change that unless in code for now
    asm volatile("sti");

    // Initialize the PIT Timer
    pit_timer_init(); // REQUIRED for the AHCI driver

    // Initialize PS2 Keyboard
    ps2_keyboard_init();

    // Initialize ACPI
    acpi_init(); // REQUIRED for userspace reboot command
    
    // Initialize PCI
    uintptr_t pci_device_tbl_addr = pci_init();

    // Initialize NVMe
    int nvme_idx = pci_find_nvme_device(); // Search for an XHCI controller
    if (nvme_idx == -ENOENT) {
        printf("[KERNEL] Could not find NVMe on the PCI bus.\n");
        kernel_finished(); // TODO KPanic here
    }
    uintptr_t nvme_bar_tbl_addr = pci_fetch_bar(nvme_idx);
    int rc = nvme_init(nvme_bar_tbl_addr);

    // Read the GUID Partition Table (LBA 0 - 33)
    uintptr_t guid_pt_buf = pmmgr_kmalloc_contiguous(5) + 0xFFFF800000000000;
    memset((uint8_t*)guid_pt_buf, 0x00, 4096 * 5);
    rc = nvme_read(guid_pt_buf, 0, 33);
    
    uint64_t hojuix_part_lba_offset = guid_pt_parse(guid_pt_buf); // Eventually make an easy searchable struct
    //uint64_t hojuix_part_lba_offset = 0;
    //ext2_init(hojuix_part_lba_offset);
    
    //printf("Hojuix Part start: %lld\n", hojuix_part_lba_offset);
    //uintptr_t elf_file = fat16_fs_test(hojuix_part_lba_offset);
    //vmmgr_map_usermode();
    //run_usermode(elf_file);

    int hda_idx = pci_find_hda_device(); // Search for an XHCI controller
    if (hda_idx == -ENOENT) {
        printf("[KERNEL] Could not find HDA on the PCI bus.\n");
        kernel_finished(); // TODO KPanic here
    }
    uintptr_t hda_bar_tbl_addr = pci_fetch_bar(hda_idx);
    hda_init(hda_bar_tbl_addr);
    printf("HDA done\n");

    uintptr_t area = pmmgr_kmalloc_contiguous(11500);
    uintptr_t area_virt = area + 0xFFFF800000000000; // Start of area
    //i386_memset((uint8_t*)pages_virt, 0x00, 4096 * 100);
    //memset((uint8_t*)(pages + 0xFFFF800000000000), 0x00, 11500*4096);
    printf("Page loc: %llx\n", area_virt);

    for (size_t i = 0; i < 90; i++) { // 91373 LBAs, read 1024 at a time to about 90 reads
        rc = nvme_read(area_virt, hojuix_part_lba_offset + (1024 * i), 1024);
        //printf("VIRT: %llx\n", area_virt);
        area_virt += 524288;
    }
    //rc = nvme_read(area_virt, hojuix_part_lba_offset, 1024);
    //rc = nvme_read(area_virt + (128 * 4096), hojuix_part_lba_offset + 1024, 1024);

    hda_play_pcm_data(area);
    /*
#if USING_AHCI
    // Find AHCI controller
    int ahci_idx = pci_find_ahci_device();
    if (ahci_idx == -ENOENT) {
        printf("[KERNEL] Could not find AHCI controller on the PCI bus.\n");
        kernel_finished(); // TODO KPanic here
    }
    uintptr_t ahci_bar_tbl_addr = pci_fetch_bar(ahci_idx);

    // Initialize the AHCI controller
    int rc = ahci_init(ahci_bar_tbl_addr);
    if (rc != EXIT_SUCCESS) {
        printf("[KERNEL] Could not initialize the AHCI controller.\n");
        kernel_finished(); // TODO KPanic here
    }

    // Parse the GUID Partition Table (LBA 0 - 33)
    int port = 0;
    uintptr_t guid_pt_buf = pmmgr_kmalloc_contiguous(5); guid_pt_buf += 0xFFFF800000000000;
    memset((uint8_t*)guid_pt_buf, 0x00, 4096 * 5);
    rc = ahci_read(port, guid_pt_buf, 0, 33);
    guid_pt_parse(guid_pt_buf);
#endif




#if XHCI
    int xhci_idx = pci_find_xhci_device(); // Search for an XHCI controller
    if (xhci_idx == -ENOENT) {
        printf("[KERNEL] Could not find XHCI controller on the PCI bus.\n");
        kernel_finished(); // TODO KPanic here
    }
    uintptr_t xhci_bar_tbl_addr = pci_fetch_bar(xhci_idx);
    xhci_init(xhci_bar_tbl_addr);
#endif

#if HDA
    //ata_pio_init();
    //fat16_fs_test();

    int hda_idx = pci_find_hda_device(); // Search for an XHCI controller
    if (hda_idx == -ENOENT) {
        printf("[KERNEL] Could not find HDA on the PCI bus.\n");
        kernel_finished(); // TODO KPanic here
    }
    uintptr_t hda_bar_tbl_addr = pci_fetch_bar(hda_idx);
    hda_init(hda_bar_tbl_addr);
    hda_play_pcm_data();
#endif
    

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

void run_usermode(uintptr_t elf_addr) {
    elf_prepare(elf_addr); // Hardcoded address from the fat16 driver
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

        /*
    printf("SPLK test start\n");
    spinlock_t sl = SPINLOCK_INIT;
    spinlock_acquire(&sl.lock);
    printf("SPLK test end\n");
    spinlock_release(&sl.lock);
    */
