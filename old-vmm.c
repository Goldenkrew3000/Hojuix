#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <kernel/drivers/rs232.h>
#include <kernel_ext/limine.h>

// I am so fucking confused.
// Like what the fuck is this...
// I just wanna do driver development, not memory management ;(




volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};




#define PTE_PRESENT	    1
#define PTE_READ_WRITE	    2
#define PTE_USER_SUPERVISOR 4
#define PTE_WRITE_THROUGH   8
#define PTE_CHACHE_DISABLED 16
#define PTE_ACCESSED	    32
#define PTE_DIRTY	    64
#define PTE_PAT		    128
#define PTE_GLOBAL	    256

#define PML4_INDEX(x) (((x) >> 39) & 0x1FF)
#define PDPT_INDEX(x) (((x) >> 30) & 0x1FF)
#define PD_INDEX(x)   (((x) >> 21) & 0x1FF)
#define PT_INDEX(x)   (((x) >> 12) & 0x1FF)

#define PAGE_SIZE 4096
#define NUM_ENTRIES 512

uint64_t* pml4 = NULL;

#define HIGHER_HALF_DATA    0xFFFF800000000000UL
#define HIGHER_HALF_CODE    0xFFFFFFFF80000000UL

static uint64_t* vmmgr_get_next_level(uint64_t* page_map_level_X, uintptr_t index_X, int flags);
void vmmgr_flush_tlb(uint64_t* address);
void vmmgr_map_page(uint64_t* current_page_dir, uintptr_t physical_addr, uintptr_t virtual_addr, int flags);


static uintptr_t higher_half_data_to_phys(uintptr_t address)
{
    return address - HIGHER_HALF_DATA;
}


uint64_t* phys_to_higher_half_data(uint64_t* address) {
    //printf("Address: %p\n", address);
    //printf("HHD: %p\n", HIGHER_HALF_DATA);
    uint64_t* ptr = (uint64_t*)((uint64_t)address + (uint64_t)HIGHER_HALF_DATA);
    //printf("LMAO %p\n", ptr);
    return ptr;
}


void vmmgr_init(void) {
    // Allocate memory for the PML4 table (Level 4 Paging 사용기)
    pml4 = (uint64_t*)pmmgr_malloc(PAGE_SIZE);
    printf("PML4 Pointer: %p\n", pml4);

    // Clear the PML4 table
    memset(phys_to_higher_half_data(pml4), 0x00, PAGE_SIZE);
    
    // Identity map the first 4GiB of memory
    for (uint64_t i = 0; i < 0x100000000; i+= PAGE_SIZE) {
        vmmgr_map_page(pml4, i, i, PTE_PRESENT | PTE_READ_WRITE);
    }

    // Map the higher half kernel address space
    //for (uint64_t i = 0; i < 0x100000000; i+= PAGE_SIZE) {
        //vmmgr_map_page(pml4, i, i + HIGHER_HALF_CODE, PTE_PRESENT | PTE_READ_WRITE);
    //}

    // Map the PMRs
    //for (uint64_t i = 0; i < 80000000; i+= PAGE_SIZE) {
        //vmmgr_map_page(pml4, i + HIGHER_HALF_CODE, i, PTE_PRESENT);
    //}
    //
    //
    //
    //
    //
    //

    // Load the new page table into CR3
    printf("Loading PML4 pointer (physical address) into CR3: %p\n", pml4);
    asm volatile("mov %0, %%cr3" : : "r" ((uint64_t)pml4) : "memory");
}


static uint64_t* vmmgr_get_next_level(uint64_t* page_map_level_X, uintptr_t index_X, int flags) {
    //printf("PMLX: %p\n", page_map_level_X);
    if (phys_to_higher_half_data(page_map_level_X)[index_X] & PTE_PRESENT) {
        //printf("HERE4\n");
        return (uint64_t*)(phys_to_higher_half_data(page_map_level_X)[index_X] & ~(511));
    } else
    {
        //printf("HERE2\n");
        phys_to_higher_half_data(page_map_level_X)[index_X] = (uint64_t)pmmgr_malloc(4096) | flags;
        //printf("HERE3\n");
        //printf("PMLX: %p\n", page_map_level_X);
        return (uint64_t*)(phys_to_higher_half_data(page_map_level_X)[index_X] & ~(511));
    }
}



void vmmgr_flush_tlb(uint64_t* address)
{
    asm volatile("invlpg (%0)" : : "r" (address) : "memory"); // This takes a virtual address
}





void vmmgr_map_page(uint64_t* current_page_dir, uintptr_t physical_addr, uintptr_t virtual_addr, int flags) {
    uintptr_t index4 = (virtual_addr & ((uintptr_t)0x1ff << 39)) >> 39;
    uintptr_t index3 = (virtual_addr & ((uintptr_t)0x1ff << 30)) >> 30;
    uintptr_t index2 = (virtual_addr & ((uintptr_t)0x1ff << 21)) >> 21;
    uintptr_t index1 = (virtual_addr & ((uintptr_t)0x1ff << 12)) >> 12;
    //printf("IDX4: %p\n", index4);
    //printf("IDX3: %p\n", index3);
    //printf("IDX2: %p\n", index2);
    //printf("IDX1: %p\n", index1); 
    //printf("CPD: %p\n", current_page_dir);

    uint64_t* page_map_level4 = current_page_dir;
    uint64_t* page_map_level3 = NULL;
    uint64_t* page_map_level2 = NULL;
    uint64_t* page_map_level1 = NULL;

    page_map_level3 = vmmgr_get_next_level(page_map_level4, index4, flags);
    //printf("PML3: %p\n", page_map_level3);
    page_map_level2 = vmmgr_get_next_level(page_map_level3, index3, flags);
    //printf("PML2: %p\n", page_map_level2);
    page_map_level1 = vmmgr_get_next_level(page_map_level2, index2, flags);
    //printf("PML1: %p\n", page_map_level1);

    phys_to_higher_half_data(page_map_level1)[index1] = physical_addr | flags;
    //printf("LMAO\n");

    // Just entertaining this
    vmmgr_flush_tlb((uint64_t*)virtual_addr);
}
