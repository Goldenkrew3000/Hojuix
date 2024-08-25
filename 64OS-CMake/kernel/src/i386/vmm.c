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


#define PML4_INDEX(x) (((x) >> 39) & 0x1FF)
#define PDPT_INDEX(x) (((x) >> 30) & 0x1FF)
#define PD_INDEX(x)   (((x) >> 21) & 0x1FF)
#define PT_INDEX(x)   (((x) >> 12) & 0x1FF)

// Grab this stuff later from actual places instead of hardcoding
uint64_t vmmgr_hhdm_offset = 0xFFFF800000000000;

void vmmgr_init() {
    // Allocate a page for each of the 4 paging structures
    pml4 = pmmgr_malloc(4096);
    //pdpt = pmmgr_malloc(4096);
    //page_directory = pmmgr_malloc(4096);
    //page_table = pmmgr_malloc(4096);
    printf("PML4 Pointer: %p\n", pml4);
    //printf("PDPT Pointer: %p\n", pdpt);
    //printf("Page Directory Pointer: %p\n", page_directory);
    //printf("Page Table Pointer: %p\n", page_table);

    // Completely clear the PML4, PDPT, Page Directory, and Page Table just to be safe
    memset(pml4, 0x00, 4096);
    //memset(pdpt, 0x00, 4096);
    //memset(page_directory, 0x00, 4096);
    //memset(page_table, 0x00, 4096);

    // Map a single page
    vmmgr_map_page(pml4, 0xffffffff00000000, 0x200000, PAGE_PRESENT | PAGE_WRITE);


    // Load the new page table
    uint64_t pml4_physical_addr = (uint64_t)pml4 - vmmgr_hhdm_offset; // Without the uint64_t cast on the uint64_t*, I get really weird subtraction results...
    printf("Loading PML4 pointer (physical address) into CR3: %p\n", pml4_physical_addr);
    asm volatile("mov %0,%%cr3" : : "r"((pml4_physical_addr)) : "memory");
    //krnl_halt();

}


uint64_t* vmmgr_get_next_level(uint64_t* current_level, uint64_t entry) {
    // I have no fucking idea what this function does...

    // Make a return pointer
    uint64_t *ret;

    if (current_level[entry] & 1) {
        // I think this means that it's the first level (PT)
        ret = (uint64_t*)(uint64_t)(current_level[entry] & ~((uint64_t)0xFFF));
    } else {
        // Alloc a block
        ret = pmmgr_malloc(4096);
        current_level[entry] = (uint64_t)ret | 0b111;
    }
    return ret;
}

void vmmgr_map_page(uint64_t current_page_dir, uint64_t virtual_addr, uint64_t physical_addr, uint64_t flags) { // I think flags should be uint8_t
    // Okay so the lower pagemaps go into the higher ones (ascending order)
    // PML1 is the only pagemap that actually contains a reference to the physical mapping (I think)

    uint64_t pagemap_entry_4 = (virtual_addr & ((uint64_t)0x1ff << 39)) >> 39; // PML4
    uint64_t pagemap_entry_3 = (virtual_addr & ((uint64_t)0x1ff << 30)) >> 30; // PDPT
    uint64_t pagemap_entry_2 = (virtual_addr & ((uint64_t)0x1ff << 21)) >> 21; // PD
    uint64_t pagemap_entry_1 = (virtual_addr & ((uint64_t)0x1ff << 12)) >> 12; // PT

    // Create pointers for the 4 pagemaps
    uint64_t* pagemap_level_4;
    uint64_t* pagemap_level_3;
    uint64_t* pagemap_level_2;
    uint64_t* pagemap_level_1;

    // So PML1 contains the physical address, and 2, 3, and 4 contain the virtual address
    // Okay, and I think current_page_dir is the pointer to the pml4?

    pagemap_level_4 = current_page_dir;
    pagemap_level_3 = vmmgr_get_next_level(pagemap_level_4, pagemap_entry_4);
    pagemap_level_2 = vmmgr_get_next_level(pagemap_level_3, pagemap_entry_3);
    pagemap_level_1 = vmmgr_get_next_level(pagemap_level_2, pagemap_entry_2);

    pagemap_level_1[pagemap_entry_1] = physical_addr | flags;
}



/*
OKAY so for 4 Level Paging (Required for x64 PAE) I need
- Page Table
- Page Directory
- Page Directory Pointer Table (PDPT)
- PML4
According to: https://www.reddit.com/r/osdev/comments/qowu8l/clarification_about_64bit_paging/
Each of these contain 512 entries, as to fill an entire page
- Apparently you only map the virtual areas you need
- For 64bit PAE, PML4 address goes into CR3 instead of the page directory
IN THE ORDER OF: PML4 --> PDPT --> PD --> PT
*/

















    // Create a blank page table
    // All paging structures need to be page-aligned addresses (4096)
    //uint64_t* page_table = pmmgr_malloc(4096);
    //printf("Page Table Location: %p\n", page_table);

    // The page directory should have exactly 512 entries (For 64bit, the entry is 8 bytes wide, so 2x what on 32bit, hence half the entries, 512 * 8 = 4096 bytes)
    // Now we need to blank the page directory. 0x00000002 -> Supervisor, Write Enabled, Not Present
    //for (int i = 0; i < 512; i++) {
        // Mark all the pages are 'Not present'. Otherwise, if the MMU finds one, it will 0xE.
    //    page_table[i] = 0x00000002;
    //}











/*
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

#define ALIGN_DOWN(A, B)    \
({                          \
    __typeof__(A) _a_ = A;      \
    __typeof__(B) _b_ = B;      \
    (_a_ / _b_) * _b_;      \
})

#define MAX(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

#define MIN(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})




extern char KERNEL_END_SYMBOL[];

pagemap_t kernel_pagemap;

uint64_t get_kernel_end_addr(void) {
    return (uint64_t) KERNEL_END_SYMBOL;
}

static uint64_t * get_next_level(uint64_t *current_level, uint64_t entry)
{
	uint64_t *ret;
	if (current_level[entry] & 1)
    {
		ret = (uint64_t *)(uint64_t)(current_level[entry] & ~((uint64_t)0xFFF));
	}
    else
    {
		ret = pmmgr_malloc(4096); // HHDM
		current_level[entry] = (uint64_t)ret | 0b111;
	}
	return ret;
}
void vmmgbdfghdsrdfgsgr_init(uint64_t virta, uint64_t physa, volatile struct limine_memmap_request memmap_request)
{
    printf("[VIRT MEM MNGR] >> Initializing VMM...\n");
    kernel_pagemap.top_level = pmmgr_malloc(4096); // HHDM
    printf("after malloc\n");
    
    for (uint64_t i = (uint64_t)256; i < 512; i++)
    {
        get_next_level(kernel_pagemap.top_level, i);
    }
    printf("im on the next level yeah\n");

    uint64_t virt = virta;
    uint64_t phys = physa;
    uint64_t len = ALIGN_UP(get_kernel_end_addr(), PAGE_SIZE) - virt;

    printf("before map\n");
    for (uint64_t j = (uint64_t)0; j < len; j += PAGE_SIZE)
    {
        vmm_map_page(&kernel_pagemap, virt + j, phys + j, 0x03);
    }
    printf("aftaur the map\n");

    printf("befaur map 2.0\n");
    //for (uint64_t i = (uint64_t)0x1000; i < 0x100000000; i += PAGE_SIZE)
    //{
    //    vmm_map_page(&kernel_pagemap, i, i, 0x03);
    //    vmm_map_page(&kernel_pagemap, i + 0xffff800000000000, i, 0x03);
    //}
    printf("aespa on da next lebeul\n");

    for (uint64_t i = 0; i < memmap_request.response->entry_count; i++)
    {
        printf("memmap loop\n");
        uint64_t base = ALIGN_DOWN(memmap_request.response->entries[i]->base, PAGE_SIZE);
        uint64_t top = ALIGN_UP(memmap_request.response->entries[i]->base + memmap_request.response->entries[i]->length, PAGE_SIZE);
        if (top <= (uint64_t)0x100000000)
        {
            continue;
        }
        for (uint64_t j = base; j < top; j += PAGE_SIZE)
        {
            if (j < (uint64_t)0x100000000)
            {
                continue;
            }
            vmm_map_page(&kernel_pagemap, j, j, 0x03);
            vmm_map_page(&kernel_pagemap, j + 0xffff800000000000, j, 0x03);
        }
    }

    printf("afta loopty loop\n");
    vmm_switch_pagemap(&kernel_pagemap);
    printf("[VIRT MEM MNGR] >> VMM Initialized\n");
}
void vmm_switch_pagemap(pagemap_t * pagemap)
{
    printf("haha1 %p\n", &pagemap);
    printf("set 없이 %p\n", pagemap->top_level);
    printf("");
    //

    uint64_t current_cr3;
    
    // Read the current value of CR3 into the variable
    asm volatile (
        "mov %%cr3, %0"
        : "=r" (current_cr3)  // Output constraint: store CR3 into current_cr3
    );
    printf("cur cr3: %p\n", current_cr3);

    uint64_t* ptr = 0x51000;

    //uint64_t cr3_value = (*pagemap->top_level - 0xFFFF800000000000) & 0x000FFFFFFFFFF000;  // Mask out reserved bits

    // Modify CR3 indirectly by setting it with the new value
    __asm__ __volatile__("mov %0,%%cr3" : : "r"(ptr) : "memory");
}
pagemap_t * vmm_new_pagemap()
{
    pagemap_t *map = pmmgr_malloc((sizeof(pagemap_t)) - 0xffff800000000000); // HHDM 없이
	map->top_level = pmmgr_malloc(4096); // HHDM

	for (size_t i = 256; i < 512; i++)
		map->top_level[i] = kernel_pagemap.top_level[i];
	return map;
}
void vmm_map_page(pagemap_t * pagemap, uint64_t virtual_address, uint64_t physical_address, uint64_t flags)
{
    uint64_t pagemap_entry_4 = (virtual_address & ((uint64_t)0x1ff << 39)) >> 39;
    uint64_t pagemap_entry_3 = (virtual_address & ((uint64_t)0x1ff << 30)) >> 30;
    uint64_t pagemap_entry_2 = (virtual_address & ((uint64_t)0x1ff << 21)) >> 21;
    uint64_t pagemap_entry_1 = (virtual_address & ((uint64_t)0x1ff << 12)) >> 12;

    uint64_t *pagemap_level_1, *pagemap_level_2, *pagemap_level_3, *pagemap_level_4;

    pagemap_level_4 = pagemap->top_level;
    pagemap_level_3 = get_next_level(pagemap_level_4, pagemap_entry_4);
    pagemap_level_2 = get_next_level(pagemap_level_3, pagemap_entry_3);
    pagemap_level_1 = get_next_level(pagemap_level_2, pagemap_entry_2);
    pagemap_level_1[pagemap_entry_1] = physical_address | flags;    
}
*/