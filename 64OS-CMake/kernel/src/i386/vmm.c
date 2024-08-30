#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <kernel_ext/limine.h>

// I am so fucking confused.
// Like what the fuck is this...
// I just wanna do driver development, not memory management ;(




volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};
extern char KERNEL_END_SYMBOL[];



/*
TEMP SERIAL
*/
#include <kernel/io.h>
#define PORT 0x3f8          // COM1

static int init_serial() {
   outb(PORT + 1, 0x00);    // Disable all interrupts
   outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(PORT + 1, 0x00);    //                  (hi byte)
   outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
   outb(PORT + 4, 0x1E);    // Set in loopback mode, test the serial chip
   outb(PORT + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

   // Check if serial is faulty (i.e: not same byte as sent)
   if(inb(PORT + 0) != 0xAE) {
      return 1;
   }

   // If serial is not faulty set it in normal operation mode
   // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
   outb(PORT + 4, 0x0F);
   return 0;
}

int is_transmit_empty() {
   return inb(PORT + 5) & 0x20;
}

void write_serial(char a) {
   while (is_transmit_empty() == 0);

   outb(PORT,a);
}

void write_line_serial(char* chars) {
    for (int i = 0; i < strlen(chars); i++) {
        write_serial(chars[i]);
    }
}




#define DIV_ROUNDUP(A, B)    \
({                           \
    __typeof__(A) _a_ = A;       \
    __typeof__(B) _b_ = B;       \
    (_a_ + (_b_ - 1)) / _b_; \
})

#define ALIGN_UP(A, B)              \
({                                  \
    __typeof__(A) _a__ = A;             \
    __typeof__(B) _b__ = B;             \
    DIV_ROUNDUP(_a__, _b__) * _b__; \
})

// Doing Level 4 Paging
// Paging Structures
uint64_t* pml4;
//uint64_t* pdpt;
//uint64_t* page_directory;
//uint64_t* page_table;

#define PAGE_PRESENT   (1ULL << 0)
#define PAGE_WRITE     (1ULL << 1)
#define PAGE_USER      (1ULL << 2)
#define PAGE_NO_EXEC   (1ULL << 63)

#define GB 0x40000000UL

#define PML4_INDEX(x) (((x) >> 39) & 0x1FF)
#define PDPT_INDEX(x) (((x) >> 30) & 0x1FF)
#define PD_INDEX(x)   (((x) >> 21) & 0x1FF)
#define PT_INDEX(x)   (((x) >> 12) & 0x1FF)

// Grab this stuff later from actual places instead of hardcoding
uint64_t vmmgr_hhdm_offset = 0xFFFF800000000000;

#define PAGE_SIZE 4096
#define NUM_ENTRIES 512


uint64_t* pml4 = NULL;

static uint64_t* vmmgr_get_next_level(uint64_t* current_level, uint64_t index);

void vmmgr_init(void) {
    // Allocate memory for the PML4 table
    pml4 = (uint64_t*)pmmgr_malloc(PAGE_SIZE);
    printf("PML4 Pointer: %p\n", pml4);

    // Clear the PML4 table
    memset(pml4, 0x00, PAGE_SIZE);
    
    // Load method based off of HyperionOS
    for (uint64_t i = 256; i < 512; i++) {
        vmmgr_get_next_level(pml4, i);
    }

    uint64_t virt = kernel_address_request.response->virtual_base;
    uint64_t phys = kernel_address_request.response->physical_base;
    uint64_t len = ALIGN_UP((uint64_t)KERNEL_END_SYMBOL, PAGE_SIZE) - virt;
    printf("LEN: %lld\n", len);

    for (uint64_t j = (uint64_t)0; j < len; j += PAGE_SIZE)
    {
        vmmgr_map_page(pml4, virt + j, phys + j, 0x03);
    }

    for (uint64_t i = (uint64_t)0x1000; i < 0x100000000; i += PAGE_SIZE)
    {
        vmmgr_map_page(pml4, i, i, 0x03);
        vmmgr_map_page(pml4, i + (uint64_t)vmmgr_hhdm_offset, i, 0x03);
    }

    // Load the new page table into CR3
    uint64_t pml4_physical_addr = (uint64_t)pml4 - (uint64_t)vmmgr_hhdm_offset;  // Assuming pml4 is already physically mapped
    printf("Loading PML4 pointer (physical address) into CR3: %p\n", pml4_physical_addr);
    asm volatile("mov %0, %%cr3" : : "r" ((uint64_t)pml4_physical_addr) : "memory");

    // Enable paging in CR0 and other necessary registers
    // (Add code to enable paging, set CR4, EFER if required)
}

static uint64_t* vmmgr_get_next_level(uint64_t* current_level, uint64_t index) {
    if (current_level[index] & PAGE_PRESENT) {
        return (uint64_t*)(current_level[index] & ~((uint64_t)0xFFF));
    } else {
        uint64_t* new_level = (uint64_t*)pmmgr_malloc(PAGE_SIZE);
        memset(new_level, 0, PAGE_SIZE);
        current_level[index] = (uint64_t)new_level | PAGE_PRESENT | PAGE_WRITE;
        return new_level;
    }
}

void vmm_flush_tlb(void *address)
{
    asm volatile("invlpg (%0)" : : "r" (address));
}

void vmmgr_map_page(uint64_t* pml4, uint64_t virtual_addr, uint64_t physical_addr, uint64_t flags) {
    uint64_t pml4_index = PML4_INDEX(virtual_addr);
    uint64_t pdpt_index = PDPT_INDEX(virtual_addr);
    uint64_t pd_index = PD_INDEX(virtual_addr);
    uint64_t pt_index = PT_INDEX(virtual_addr);

     //printf("Mapping virtual address %p to physical address", virtual_addr);
     //printf(" %p\n", physical_addr);

    uint64_t* pdpt = vmmgr_get_next_level(pml4, pml4_index);
    uint64_t* pd = vmmgr_get_next_level(pdpt, pdpt_index);
    uint64_t* pt = vmmgr_get_next_level(pd, pd_index);

    pt[pt_index] = physical_addr | flags;

    // Just entertaining this
    vmm_flush_tlb((void *)virtual_addr);

    //asm volatile("nop");
}