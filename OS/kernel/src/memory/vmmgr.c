#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <kernel.h>
#include <kernel/memory/pmmgr.h>
#include <kernel/memory/vmmgr.h>
#include <kernel_ext/limine.h>

extern uint64_t KERNEL_START_SYM[];
extern uint64_t KERNEL_WRITE_SYM[];
extern uint64_t KERNEL_END_SYM[];
uint64_t kernel_start = (uint64_t)KERNEL_START_SYM;
uint64_t writeallowed_start = (uint64_t)KERNEL_WRITE_SYM;
uint64_t kernel_end = (uint64_t)KERNEL_END_SYM;

void vmmgr_init() {
    printf("[VMMGR] Initializing...\n");
    uint64_t pml4_virt = ((uint64_t)pmmgr_kmalloc(1)) + kerndata.hhdm_offset;
    kern_memset((uint8_t*)pml4_virt, 0, 4096);
    vmmgr_map_all((uint64_t*)pml4_virt);
    kerndata.cr3 = pml4_virt - kerndata.hhdm_offset;
}

// Returns the physical address of a virtual address.
// Returns 0xDEAD if it's unmapped.
uint64_t vmmgr_virt_to_phys(uint64_t pml4_addr[], uint64_t virt_addr) {
    virt_addr &= ~TOPBITS;
    uint64_t pml1 = (virt_addr >> 12) & 511;
    uint64_t pml2 = (virt_addr >> (12 + 9)) & 511;
    uint64_t pml3 = (virt_addr >> (12 + 18)) & 511;
    uint64_t pml4 = (virt_addr >> (12 + 27)) & 511;
    for (; pml4 < 512; pml4++) {
        uint64_t *pml3_addr = NULL;
        if (pml4_addr[pml4] == 0)
            return 0xDEAD;
        else
            pml3_addr = (uint64_t*)(PAGE_ALIGN_DOWN(pml4_addr[pml4]) + kerndata.hhdm_offset);

        for (; pml3 < 512; pml3++) {
            uint64_t *pml2_addr = NULL;
            if (pml3_addr[pml3] == 0)
                return 0xDEAD;
            else
                pml2_addr = (uint64_t*)(PAGE_ALIGN_DOWN(pml3_addr[pml3]) + kerndata.hhdm_offset);

            for (; pml2 < 512; pml2++) {
                uint64_t *pml1_addr = NULL;
                if (pml2_addr[pml2] == 0)
                    return 0xDEAD;
                else
                    pml1_addr = (uint64_t*)(PAGE_ALIGN_DOWN(pml2_addr[pml2]) + kerndata.hhdm_offset);
                
                if (pml1_addr[pml1] == 0)
                    return 0xDEAD;

                return (uint64_t)(PAGE_ALIGN_DOWN(pml1_addr[pml1]) + (virt_addr - PAGE_ALIGN_DOWN(virt_addr)));
            }
            pml2 = 0;
        }
        pml3 = 0;
    }
    return 0xDEAD;
}

// writes data to a location in virtual memory
void vmmgr_write_vmem(uint64_t *pml4_addr, uint64_t virt_addr, char *data, size_t len) {
    while (len > 0) {
        // get the address of this virtual address in kernel memory
        uint64_t kernel_addr = vmmgr_virt_to_phys(pml4_addr, virt_addr);
        if (kernel_addr == 0xDEAD) {
            printf("virtual address is not mapped (write_vmem), address: %p\n", virt_addr);
            asm volatile("hlt");
        }
        kernel_addr += kerndata.hhdm_offset;
        uint64_t bytes_to_copy = (len < PAGE_SIZE) ? len : PAGE_SIZE;
        kern_memcpy((char*) kernel_addr, data, bytes_to_copy);
        len -= bytes_to_copy;
        virt_addr += bytes_to_copy;
        data += bytes_to_copy;
    }
}

// pushes data onto another stack in another virtual memory address tree thingy
void vmmgr_push_vmem(uint64_t *pml4_addr, uint64_t rsp, char *data, size_t len) {
    rsp -= len;
    vmmgr_write_vmem(pml4_addr, rsp, data, len);
}

void vmmgr_map_pages(uint64_t pml4_addr[], uint64_t virt_addr, uint64_t phys_addr, uint64_t num_pages, uint64_t flags) {
    virt_addr &= ~TOPBITS;
    uint64_t pml1 = (virt_addr >> 12) & 511;
    uint64_t pml2 = (virt_addr >> (12 + 9)) & 511;
    uint64_t pml3 = (virt_addr >> (12 + 18)) & 511;
    uint64_t pml4 = (virt_addr >> (12 + 27)) & 511;
    for (; pml4 < 512; pml4++) {
        uint64_t *pml3_addr = NULL;
        if (pml4_addr[pml4] == 0) {
            pml4_addr[pml4] = (uint64_t)pmmgr_kmalloc(1);
            pml3_addr = (uint64_t*)(pml4_addr[pml4] + kerndata.hhdm_offset);
            kern_memset((uint8_t*)pml3_addr, 0, 4096);
            pml4_addr[pml4] |= flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER;
        } else {
            pml3_addr = (uint64_t*)(PAGE_ALIGN_DOWN(pml4_addr[pml4]) + kerndata.hhdm_offset);
        }
        
        for (; pml3 < 512; pml3++) {
            uint64_t *pml2_addr = NULL;
            if (pml3_addr[pml3] == 0) {
                pml3_addr[pml3] = (uint64_t)pmmgr_kmalloc(1);
                pml2_addr = (uint64_t*)(pml3_addr[pml3] + kerndata.hhdm_offset);
                kern_memset((uint8_t*)pml2_addr, 0, 4096);
                pml3_addr[pml3] |= flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER;
            } else {
                pml2_addr = (uint64_t*)(PAGE_ALIGN_DOWN(pml3_addr[pml3]) + kerndata.hhdm_offset);
            }

            for (; pml2 < 512; pml2++) {
                uint64_t *pml1_addr = NULL;
                if (pml2_addr[pml2] == 0) {
                    pml2_addr[pml2] = (uint64_t)pmmgr_kmalloc(1);
                    pml1_addr = (uint64_t*)(pml2_addr[pml2] + kerndata.hhdm_offset);
                    kern_memset((uint8_t*)pml1_addr, 0, 4096);
                    pml2_addr[pml2] |= flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER;
                } else {
                    pml1_addr = (uint64_t*)(PAGE_ALIGN_DOWN(pml2_addr[pml2]) + kerndata.hhdm_offset);
                }
                for (; pml1 < 512; pml1++) {
                    pml1_addr[pml1] = phys_addr | flags;
                    num_pages--;
                    phys_addr += 4096;
                    if (num_pages == 0) return;
                }
                pml1 = 0;
            }
            pml2 = 0;
        }
        pml3 = 0;
    }
    printf("no more virt mem\n");
    asm volatile("hlt");
} 

void vmmgr_alloc_pages(uint64_t pml4_addr[], uint64_t virt_addr, uint64_t num_pages, uint64_t flags) {
    virt_addr &= ~TOPBITS;
    uint64_t pml1 = (virt_addr >> 12) & 511;
    uint64_t pml2 = (virt_addr >> (12 + 9)) & 511;
    uint64_t pml3 = (virt_addr >> (12 + 18)) & 511;
    uint64_t pml4 = (virt_addr >> (12 + 27)) & 511;
    for (; pml4 < 512; pml4++) {
        uint64_t *pml3_addr = NULL;
        if (pml4_addr[pml4] == 0) {
            pml4_addr[pml4] = (uint64_t)pmmgr_kmalloc(1);
            pml3_addr = (uint64_t*)(pml4_addr[pml4] + kerndata.hhdm_offset);
            kern_memset((uint8_t*)pml3_addr, 0, 4096);
            pml4_addr[pml4] |= flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER;
        } else {
            pml3_addr = (uint64_t*)(PAGE_ALIGN_DOWN(pml4_addr[pml4]) + kerndata.hhdm_offset);
        }
        for (; pml3 < 512; pml3++) {
            uint64_t *pml2_addr = NULL;
            if (pml3_addr[pml3] == 0) {
                pml3_addr[pml3] = (uint64_t)pmmgr_kmalloc(1);
                pml2_addr = (uint64_t*)(pml3_addr[pml3] + kerndata.hhdm_offset);
                kern_memset((uint8_t*)pml2_addr, 0, 4096);
                pml3_addr[pml3] |= flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER;
            } else {
                pml2_addr = (uint64_t*)(PAGE_ALIGN_DOWN(pml3_addr[pml3]) + kerndata.hhdm_offset);
            }

            for (; pml2 < 512; pml2++) {
                uint64_t *pml1_addr = NULL;
                if (pml2_addr[pml2] == 0) {
                    pml2_addr[pml2] = (uint64_t)pmmgr_kmalloc(1);
                    pml1_addr = (uint64_t*)(pml2_addr[pml2] + kerndata.hhdm_offset);
                    kern_memset((uint8_t*)pml1_addr, 0, 4096);
                    pml2_addr[pml2] |= flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER;
                } else {
                    pml1_addr = (uint64_t*)(PAGE_ALIGN_DOWN(pml2_addr[pml2]) + kerndata.hhdm_offset);
                }
                
                for (; pml1 < 512; pml1++) {
                    uint64_t phys = (uint64_t)pmmgr_kmalloc(1);
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
    printf("no more virtual memory\n");
    asm volatile("hlt");
}

void vmmgr_map_sections(uint64_t pml4[]) {
    uint64_t num_memmap_entries                = kerndata.memmap.entry_count;
    struct limine_memmap_entry *memmap_entries = *kerndata.memmap.entries;
    for (size_t entry = 0; entry < num_memmap_entries; entry++) {
        uint64_t entry_type = memmap_entries[entry].type;
        if (entry_type == LIMINE_MEMMAP_USABLE ||
            entry_type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE ||
            entry_type == LIMINE_MEMMAP_FRAMEBUFFER ||
            entry_type == LIMINE_MEMMAP_KERNEL_AND_MODULES ||
            entry_type == LIMINE_MEMMAP_ACPI_NVS ||
            entry_type == LIMINE_MEMMAP_ACPI_RECLAIMABLE) {
            vmmgr_map_pages(pml4, memmap_entries[entry].base + kerndata.hhdm_offset, memmap_entries[entry].base, memmap_entries[entry].length / 4096, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE);
        }
    }
}

void vmmgr_map_kernel(uint64_t pml4[]) {
    uint64_t length_buffer = 0;
    uint64_t phys_buffer = 0;
    /* map from kernel_start to writeallowed_start with only the present flag */
    length_buffer = PAGE_ALIGN_UP(writeallowed_start - kernel_start);
    phys_buffer = kerndata.kernel_addr.physical_base + (kernel_start - kerndata.kernel_addr.virtual_base);
    vmmgr_map_pages(pml4, PAGE_ALIGN_DOWN(kernel_start), phys_buffer, length_buffer / 4096, KERNEL_PFLAG_PRESENT);
    /* map from writeallowed_start to kernel_end with `present` and `write` flags */
    length_buffer = PAGE_ALIGN_UP(kernel_end - writeallowed_start);
    phys_buffer = kerndata.kernel_addr.physical_base + (writeallowed_start - kerndata.kernel_addr.virtual_base);
    vmmgr_map_pages(pml4, PAGE_ALIGN_DOWN(writeallowed_start), phys_buffer, length_buffer / 4096, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE); 
    // map the kernel's stack
    vmmgr_alloc_pages(pml4, KERNEL_STACK_ADDR, KERNEL_STACK_PAGES, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE);
}

void vmmgr_map_all(uint64_t pml4[]) {
    vmmgr_map_kernel(pml4);
    vmmgr_map_sections(pml4);
}
