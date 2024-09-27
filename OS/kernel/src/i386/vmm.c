#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <kernel/drivers/rs232.h>
#include <kernel_ext/limine.h>

volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

extern uint64_t KERNEL_START_SYM[];
extern uint64_t KERNEL_WRITE_SYM[];
extern uint64_t KERNEL_END_SYM[];
uint64_t kernel_start = (uint64_t)KERNEL_START_SYM;
uint64_t kernel_write = (uint64_t)KERNEL_WRITE_SYM;
uint64_t kernel_end = (uint64_t)KERNEL_END_SYM;

#define PAGE_ALIGN_DOWN(addr) ((addr / 4096) * 4096)
#define PAGE_ALIGN_UP(x) ((((x) + 4095) / 4096) * 4096)

#define KERNEL_PFLAG_PRESENT 0b1
#define KERNEL_PFLAG_WRITE   0b10
#define KERNEL_PFLAG_USER    0b100
#define KERNEL_PFLAG_PXD     0b10000000000000000000000000000000000000000000000000000000000000

#define PAGE_SIZE 4096
#define KERNEL_STACK_PAGES 2LL
#define KERNEL_STACK_PTR 0xFFFFFFFFFFFFF000LL
#define KERNEL_STACK_ADDR KERNEL_STACK_PTR-(KERNEL_STACK_PAGES*PAGE_SIZE)

#define HHDM_OFFSET 0xFFFF800000000000
#define TOPBITS 0xFFFF000000000000

/*
uint64_t* phys_to_higher_half_data(uint64_t* address) {
    //printf("Address: %p\n", address);
    //printf("HHD: %p\n", HIGHER_HALF_DATA);
    uint64_t* ptr = (uint64_t*)((uint64_t)address + (uint64_t)HIGHER_HALF_DATA);
    //printf("LMAO %p\n", ptr);
    return ptr;
}
*/


void vmmgr_init() {
    // Allocate memory for the PML4 table (Level 4 Paging 사용기)
    uint64_t pml4 = (uint64_t)pmmgr_malloc(PAGE_SIZE) + HHDM_OFFSET; // Physical Address
    printf("PML4 Pointer: %p\n", pml4);

    // Clear the PML4 table
    memset((uint8_t*)pml4, 0x00, 4096);

    // Map memory
    vmmgr_map_everything((uint64_t*)pml4);

    // Load the new page table into CR3
    uint64_t cr3_pml4 = pml4 - HHDM_OFFSET;
    printf("Loading PML4 pointer (physical address) into CR3: %p\n", cr3_pml4);
    //asm volatile("movq %0, %%cr3" : : "r" (cr3_pml4));
    
    // Load new kernel stack pointer
    //asm volatile("movq %0, %%rsp; movq $0, %%rbp; push $0" : : "r" (KERNEL_STACK_PTR));
}

void vmmgr_map_everything(uint64_t pml4[]) {
    vmmgr_map_kernel(pml4);
    vmmgr_map_memory(pml4);
}

void vmmgr_map_kernel(uint64_t pml4[]) {
    uint64_t length_buffer = 0;
    uint64_t physical_buffer = 0;

    // Map from KERNEL_START to KERNEL_WRITE with PRESENT
    length_buffer = PAGE_ALIGN_UP(kernel_write - kernel_start);
    physical_buffer = kernel_address_request.response->physical_base + (kernel_start - kernel_address_request.response->virtual_base);
    vmmgr_map_page(pml4, PAGE_ALIGN_DOWN(kernel_start), physical_buffer, length_buffer / 4096, KERNEL_PFLAG_PRESENT);

    // Map from KERNEL_WRITE to KERNEL_END with PRESENT and WRITE
    length_buffer = PAGE_ALIGN_UP(kernel_end - kernel_write);
    physical_buffer = kernel_address_request.response->physical_base + (kernel_write - kernel_address_request.response->virtual_base);
    vmmgr_map_page(pml4, PAGE_ALIGN_DOWN(kernel_write), physical_buffer, length_buffer / 4096, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE);

    // Map the kernel stack
    vmmgr_alloc_page(pml4, KERNEL_STACK_ADDR, KERNEL_STACK_PAGES, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE);
}

/*
length_buffer = PAGE_ALIGN_UP(kernel_end - writeallowed_start);
    phys_buffer = kernel.kernel_addr.physical_base + (writeallowed_start - kernel.kernel_addr.virtual_base);
    map_pages(pml4, PAGE_ALIGN_DOWN(writeallowed_start), phys_buffer, length_buffer / 4096, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE); 
    // map the kernel's stack
    alloc_pages(pml4, KERNEL_STACK_ADDR, KERNEL_STACK_PAGES, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE);
}
*/

void vmmgr_map_page(uint64_t pml4_addr[], uint64_t virtual_addr, uint64_t physical_addr, uint64_t num_pages, uint64_t flags) {
    virtual_addr &= ~TOPBITS;
    uint64_t pml1 = (virtual_addr >> 12) & 511;
    uint64_t pml2 = (virtual_addr >> (12 + 9)) & 511;
    uint64_t pml3 = (virtual_addr >> (12 + 18)) & 511;
    uint64_t pml4 = (virtual_addr >> (12 + 27)) & 511;

    for (; pml4 < 512; pml4++) {
        uint64_t *pml3_addr = NULL;
        if (pml4_addr[pml4] == 0) {
            pml4_addr[pml4] = (uint64_t)pmmgr_malloc(4096);
            pml3_addr = (uint64_t*)(pml4_addr[pml4] + HHDM_OFFSET);
            memset((uint8_t*)pml3_addr, 0, 4096);
            pml4_addr[pml4] |= flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE;
        } else {
            pml3_addr = (uint64_t*)(PAGE_ALIGN_DOWN(pml4_addr[pml4]) + HHDM_OFFSET);
        }
        
        for (; pml3 < 512; pml3++) {
            uint64_t *pml2_addr = NULL;
            if (pml3_addr[pml3] == 0) {
                pml3_addr[pml3] = (uint64_t)pmmgr_malloc(4096);
                pml2_addr = (uint64_t*)(pml3_addr[pml3] + HHDM_OFFSET);
                memset((uint8_t*)pml2_addr, 0, 4096);
                pml3_addr[pml3] |= flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE;
            } else {
                pml2_addr = (uint64_t*)(PAGE_ALIGN_DOWN(pml3_addr[pml3]) + HHDM_OFFSET);
            }

            for (; pml2 < 512; pml2++) {
                uint64_t *pml1_addr = NULL;
                if (pml2_addr[pml2] == 0) {
                    pml2_addr[pml2] = (uint64_t)pmmgr_malloc(4096);
                    pml1_addr = (uint64_t*)(pml2_addr[pml2] + HHDM_OFFSET);
                    memset((uint8_t*)pml1_addr, 0, 4096);
                    pml2_addr[pml2] |= flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE;
                } else {
                    pml1_addr = (uint64_t*)(PAGE_ALIGN_DOWN(pml2_addr[pml2]) + HHDM_OFFSET);
                }
                for (; pml1 < 512; pml1++) {
                    pml1_addr[pml1] = physical_addr | flags;
                    num_pages--;
                    physical_addr += 4096;
                    if (num_pages == 0) return;
                }
                pml1 = 0;
            }
            pml2 = 0;
        }
        pml3 = 0;
    }

    // Out of memory
    printf("haha fuck u stupid bad computer u hab no memory :3\n"); // TODO
}

void vmmgr_alloc_page(uint64_t pml4_addr[], uint64_t virtual_addr, uint64_t num_pages, uint64_t flags) {
    virtual_addr &= ~TOPBITS;
    uint64_t pml1 = (virtual_addr >> 12) & 511;
    uint64_t pml2 = (virtual_addr >> (12 + 9)) & 511;
    uint64_t pml3 = (virtual_addr >> (12 + 18)) & 511;
    uint64_t pml4 = (virtual_addr >> (12 + 27)) & 511;
    for (; pml4 < 512; pml4++) {
        uint64_t *pml3_addr = NULL;
        if (pml4_addr[pml4] == 0) {
            pml4_addr[pml4] = (uint64_t)pmmgr_malloc(4096);
            pml3_addr = (uint64_t*)(pml4_addr[pml4] + HHDM_OFFSET);
            memset((uint8_t*)pml3_addr, 0, 4096);
            pml4_addr[pml4] |= flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE;
        } else {
            pml3_addr = (uint64_t*)(PAGE_ALIGN_DOWN(pml4_addr[pml4]) + HHDM_OFFSET);
        }
        for (; pml3 < 512; pml3++) {
            uint64_t *pml2_addr = NULL;
            if (pml3_addr[pml3] == 0) {
                pml3_addr[pml3] = (uint64_t)pmmgr_malloc(4096);
                pml2_addr = (uint64_t*)(pml3_addr[pml3] + HHDM_OFFSET);
                memset((uint8_t*)pml2_addr, 0, 4096);
                pml3_addr[pml3] |= flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE;
            } else {
                pml2_addr = (uint64_t*)(PAGE_ALIGN_DOWN(pml3_addr[pml3]) + HHDM_OFFSET);
            }

            for (; pml2 < 512; pml2++) {
                uint64_t *pml1_addr = NULL;
                if (pml2_addr[pml2] == 0) {
                    pml2_addr[pml2] = (uint64_t)pmmgr_malloc(4096);
                    pml1_addr = (uint64_t*)(pml2_addr[pml2] + HHDM_OFFSET);
                    memset((uint8_t*)pml1_addr, 0, 4096);
                    pml2_addr[pml2] |= flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE;
                } else {
                    pml1_addr = (uint64_t*)(PAGE_ALIGN_DOWN(pml2_addr[pml2]) + HHDM_OFFSET);
                }
                
                for (; pml1 < 512; pml1++) {
                    uint64_t phys = (uint64_t)pmmgr_malloc(4096);
                    pml1_addr[pml1] = phys | flags;
                    num_pages--;
                    if (num_pages == 0) return;
                }
                pml1 = 0;
            }
            pml2 = 0;
        }
        pml3 = 0;
    }
    
    // Out of memory
    printf("oopsy poopsy\n"); // TODO
}

void vmmgr_flush_tlb(uint64_t* address)
{
    asm volatile("invlpg (%0)" : : "r" (address) : "memory"); // This takes a virtual address
}
