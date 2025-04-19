#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <kernel.h>
#include <kernel/memory/pmmgr.h>
#include <kernel/memory/vmmgr.h>
#include <kernel/drivers/rs232.h>
#include <kernel_ext/limine.h>

extern char kern_text_start_addr[];
extern char kern_text_end_addr[];
extern char kern_rodata_start_addr[];
extern char kern_rodata_end_addr[];
extern char kern_data_start_addr[];
extern char kern_data_end_addr[];

extern uint32_t* i386_kern_memset();

uint64_t hhdm_off = 0xFFFF800000000000;

// keep build happy
//uint64_t kernel_start;
//uint64_t writeallowed_start;
//uint64_t kernel_end;

extern uint64_t KERNEL_START_SYM[];
extern uint64_t KERNEL_WRITE_SYM[];
extern uint64_t KERNEL_END_SYM[];
uint64_t kernel_start = (uint64_t)KERNEL_START_SYM;
uint64_t writeallowed_start = (uint64_t)KERNEL_WRITE_SYM;
uint64_t kernel_end = (uint64_t)KERNEL_END_SYM;

uintptr_t pml4_global;

uintptr_t vmmgr_kalloc_page(uintptr_t virt_addr, int pages) {
    //uintptr_t page = (uintptr_t)pmmgr_kmalloc(1) + (uintptr_t)hhdm_off;
    vmmgr_alloc_pages((uint64_t*)pml4_global, virt_addr, pages, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE);
}

// INFO: This is used for debugging if a physical address (calculated to a virtual address) is on the TLB
// In the case of ACPI: No, it returned 0xdead on real hardware, but the same phys address in an emulator
// Effectively, debugging the TLB on real hardware to find the root cause of a page fault
uintptr_t vmmgr_virt_to_phys_ext(uintptr_t virt_addr) {
    uint64_t res = vmmgr_virt_to_phys((uint64_t*)pml4_global, virt_addr);
    return (uintptr_t)res;
}








//extern uint8_t _binary____usermode_test_start[], _binary____usermode_test_end[];
extern uint8_t* i386_kern_memcpy();

void vmmgr_map_usermode() {
    // Allocate and map usermode stack
    vmmgr_alloc_pages((uint64_t*)pml4_global, USER_STACK_ADDR, 3, KERNEL_PFLAG_USER | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE);
    printf("Mapped usermode stack\n");


    // Allocate a usermode data page
    uintptr_t usermode_data_page = 0x6050000000;
    vmmgr_alloc_pages((uint64_t*)pml4_global, usermode_data_page, 1, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER);
    printf("Mapped usermode data page\n");

    // Allocate a usermode executable page
    //uintptr_t usermode_exec_page = 0x6000000000;
    uintptr_t usermode_exec_page = 0x400000;
    //asm volatile("invlpg (%0)" : : "r" (usermode_exec_page) : "memory");
    vmmgr_alloc_pages((uint64_t*)pml4_global, usermode_exec_page, 1, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER); // USER as well
    //vmmgr_alloc_pages((uint64_t*)pml4_global, usermode_exec_page, 1, KERNEL_PFLAG_ALL);
    //usermode_exec_page = (uintptr_t)pmmgr_kmalloc(1);
    //printf("EXEC PAGE: %llx\n", usermode_exec_page);
    printf("Mapped usermode exec page\n");

    // Blank pages
    i386_kern_memset((uint8_t*)usermode_data_page, 0x00, 4096);
    i386_kern_memset((uint8_t*)usermode_exec_page, 0x00, 4096);

    // Copy usermode test program from kernel mapped pages into the user exec page
    // NOTE: This all seems to match up (bytes wise) to the objdump version
    //size_t user_code_size = _binary____usermode_test_end - _binary____usermode_test_start;
    //printf("(In kernel page) User program start: %llx\n", _binary____usermode_test_start);
    //printf("(In kernel page) User program start: %llx\n", _binary____usermode_test_end);
    //printf("(In kernel page) User program size (in bytes): %lld\n", user_code_size);
    //i386_kern_memcpy((uint8_t*)usermode_exec_page, (uint8_t*)_binary____usermode_test_start, user_code_size);
    //uint8_t* usermode_exec_page_access = (uint8_t*)usermode_exec_page;


    unsigned char usermode_exec_page_access_raw[] = {
        0x55, 0x48, 0x89, 0xe5, 0x89, 0x7d, 0xfc, 0x48, 0x89, 0x75, 0xf0, 0xbb,
        0x01, 0x00, 0x00, 0x00, 0xb9, 0x48, 0x00, 0x00, 0x00, 0xcd, 0x80, 0xbb,
        0x01, 0x00, 0x00, 0x00, 0xb9, 0x64, 0x00, 0x00, 0x00, 0xcd, 0x80, 0xbb,
        0x02, 0x00, 0x00, 0x00, 0xcd, 0x80, 0xb8, 0x0a, 0x00, 0x00, 0x00, 0x5d,
        0xc3
      };





    i386_kern_memcpy((uint8_t*)usermode_exec_page, &usermode_exec_page_access_raw, 49);
    printf("success\n");

    // Sanity check the page
    //uint8_t* ueps = (uint8_t*)usermode_exec_page; // user exec page sanity
    //printf("Sanity check: %02X %02X %02X %02X %02X %02X\n", ueps[0], ueps[1], ueps[2], ueps[3], ueps[4], ueps[5]); // WOOOO
}



void bp() {return;}



void vmmgr_init() {
    //printf("[VMMGR] Initializing...\n");
    //rs232_writeline(1, "[VMMGR] Initializing...\r\n");

	// Malloc and zero PML4 page
    uintptr_t pml4_virt = (uintptr_t)pmmgr_kmalloc(1) + (uintptr_t)hhdm_off;
    pml4_global = pml4_virt;
    //memset((void*)pml4_virt, 0x00, 4096);
    //kern_memset((uint8_t*)pml4_virt, 0, 4096);
    i386_kern_memset((void*)pml4_virt, 0x00, 4096);

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
            i386_kern_memset((uint8_t*)pml3_addr, 0x00, 4096);
            bp();
            //pml4_addr[pml4] |= KERNEL_PFLAG_ALL;
            pml4_addr[pml4] |= flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER;
        } else {
            pml3_addr = (uint64_t*)(PAGE_ALIGN_DOWN(pml4_addr[pml4]) + kerndata.hhdm_offset);
        }

        for (; pml3 < 512; pml3++) {
            uint64_t *pml2_addr = NULL;
            if (pml3_addr[pml3] == 0) {
                pml3_addr[pml3] = (uint64_t)pmmgr_kmalloc(1);
                pml2_addr = (uint64_t*)(pml3_addr[pml3] + kerndata.hhdm_offset);
                i386_kern_memset((uint8_t*)pml2_addr, 0x00, 4096);
                //pml3_addr[pml3] |= KERNEL_PFLAG_ALL;
                pml3_addr[pml3] |= flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER;
            } else {
                pml2_addr = (uint64_t*)(PAGE_ALIGN_DOWN(pml3_addr[pml3]) + kerndata.hhdm_offset);
            }

            for (; pml2 < 512; pml2++) {
                uint64_t *pml1_addr = NULL;
                if (pml2_addr[pml2] == 0) {
                    pml2_addr[pml2] = (uint64_t)pmmgr_kmalloc(1);
                    pml1_addr = (uint64_t*)(pml2_addr[pml2] + kerndata.hhdm_offset);
                    i386_kern_memset((uint8_t*)pml1_addr, 0x00, 4096);
                    //pml2_addr[pml2] |= KERNEL_PFLAG_ALL;
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
            i386_kern_memset((uint8_t*)pml3_addr, 0x00, 4096);
            //pml4_addr[pml4] |= KERNEL_PFLAG_ALL;
            pml4_addr[pml4] |= flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER;
        } else {
            pml3_addr = (uint64_t*)(PAGE_ALIGN_DOWN(pml4_addr[pml4]) + kerndata.hhdm_offset);
        }
        for (; pml3 < 512; pml3++) {
            uint64_t *pml2_addr = NULL;
            if (pml3_addr[pml3] == 0) {
                pml3_addr[pml3] = (uint64_t)pmmgr_kmalloc(1);
                pml2_addr = (uint64_t*)(pml3_addr[pml3] + kerndata.hhdm_offset);
                i386_kern_memset((uint8_t*)pml2_addr, 0x00, 4096);
                //pml3_addr[pml3] |= KERNEL_PFLAG_ALL;
                pml3_addr[pml3] |= flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER;
            } else {
                pml2_addr = (uint64_t*)(PAGE_ALIGN_DOWN(pml3_addr[pml3]) + kerndata.hhdm_offset);
            }

            for (; pml2 < 512; pml2++) {
                uint64_t *pml1_addr = NULL;
                if (pml2_addr[pml2] == 0) {
                    pml2_addr[pml2] = (uint64_t)pmmgr_kmalloc(1);
                    pml1_addr = (uint64_t*)(pml2_addr[pml2] + kerndata.hhdm_offset);
                    i386_kern_memset((uint8_t*)pml1_addr, 0x00, 4096);
                    //pml2_addr[pml2] |= KERNEL_PFLAG_ALL;
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

    // ISSUE: On my thinkpad, the ACPI XSDT table is inside ACPI, but the FACP/etc tables are in a RESERVED patch!?!??!
    // FIX: For now, map RESERVED memory as present but only read. Future fix could to to map it if it's not on the pagemap (I can do that: I know length, where, and can check the TLB)
    // But I am planning on reworking the paging code, and that would be a relatively major thing to implement
    for (size_t entry = 0; entry < num_memmap_entries; entry++) {
        uint64_t entry_type = memmap_entries[entry].type;
        if (entry_type == LIMINE_MEMMAP_RESERVED) {
            vmmgr_map_pages(pml4, memmap_entries[entry].base + kerndata.hhdm_offset, memmap_entries[entry].base, memmap_entries[entry].length / 4096, KERNEL_PFLAG_PRESENT);
        }
    }
}

void vmmgr_map_kernel(uint64_t pml4[]) {
    //printf("Kernel Phys Start: %llx\n", kerndata.kernel_addr.physical_base);
    //printf("Kernel Virt Start: %llx\n", kerndata.kernel_addr.virtual_base);
    //printf("Kern page align down: %llx\n", PAGE_ALIGN_DOWN(kernel_start));

    /*
    uint64_t length_buffer = 0;
    uint64_t phys_buffer = 0;
    // map from kernel_start to writeallowed_start with only the present flag
    length_buffer = PAGE_ALIGN_UP(writeallowed_start - kernel_start);
    phys_buffer = kerndata.kernel_addr.physical_base + (kernel_start - kerndata.kernel_addr.virtual_base);
    //printf("Kern page align down: %llx --- %llx\n", PAGE_ALIGN_DOWN(kernel_start), phys_buffer);
    vmmgr_map_pages(pml4, PAGE_ALIGN_DOWN(kernel_start), phys_buffer, length_buffer / 4096, KERNEL_PFLAG_PRESENT);
    // map from writeallowed_start to kernel_end with `present` and `write` flags
    length_buffer = PAGE_ALIGN_UP(kernel_end - writeallowed_start);
    phys_buffer = kerndata.kernel_addr.physical_base + (writeallowed_start - kerndata.kernel_addr.virtual_base);
    vmmgr_map_pages(pml4, PAGE_ALIGN_DOWN(writeallowed_start), phys_buffer, length_buffer / 4096, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE);
    // map the kernel's stack
    vmmgr_alloc_pages(pml4, KERNEL_STACK_ADDR, KERNEL_STACK_PAGES, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE);
    */

    //uint64_t length_buffer = 0;
    //uint64_t phys_buffer = 0;
    //length_buffer = PAGE_ALIGN_UP(kernel_end - kernel_start);
    //phys_buffer = kerndata.kernel_addr.physical_base + (kernel_start - kerndata.kernel_addr.virtual_base);
    //printf("Kern page align down: %llx --- %llx\n", PAGE_ALIGN_DOWN(kernel_start), phys_buffer);
    //printf("Virt start: %llx\n", PAGE_ALIGN_DOWN(kernel_start));
    //printf("Phys start: %llx\n", phys_buffer);
    //vmmgr_map_pages(pml4, PAGE_ALIGN_DOWN(kernel_start), phys_buffer, length_buffer / 4096, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_EXEC);


    uintptr_t text_start = PAGE_ALIGN_DOWN((uintptr_t)kern_text_start_addr);
    uintptr_t rodata_start = PAGE_ALIGN_DOWN((uintptr_t)kern_rodata_start_addr);
	uintptr_t data_start = PAGE_ALIGN_DOWN((uintptr_t)kern_data_start_addr);
	uintptr_t text_end = PAGE_ALIGN_UP((uintptr_t)kern_text_end_addr);
	uintptr_t rodata_end = PAGE_ALIGN_UP((uintptr_t)kern_rodata_end_addr);
	uintptr_t data_end = PAGE_ALIGN_UP((uintptr_t)kern_data_end_addr);

	uint64_t paddr = kerndata.kernel_addr.physical_base;
	uint64_t vaddr = kerndata.kernel_addr.virtual_base;

	for (uintptr_t text_addr = text_start; text_addr < text_end;
			 text_addr += PAGE_SIZE) {
			uintptr_t phys = text_addr - vaddr + paddr;
			vmmgr_map_pages(pml4, text_addr, phys, 1, 1);
		}

		for (uintptr_t rodata_addr = rodata_start; rodata_addr < rodata_end;
			 rodata_addr += PAGE_SIZE) {
			uintptr_t phys = rodata_addr - vaddr + paddr;
			vmmgr_map_pages(pml4, rodata_addr, phys, 1, 1);// | 1ull << 63ull);
		}

		for (uintptr_t data_addr = data_start; data_addr < data_end;
			 data_addr += PAGE_SIZE) {
			uintptr_t phys = data_addr - vaddr + paddr;
			vmmgr_map_pages(pml4, data_addr, phys, 1, 0b11);// | 1ull << 63ull);
		}


		// map le stack
		vmmgr_alloc_pages(pml4, KERNEL_STACK_ADDR, KERNEL_STACK_PAGES, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE);
}

void vmmgr_map_all(uint64_t pml4[]) {
    vmmgr_map_kernel(pml4);
    vmmgr_map_sections(pml4);
}

void vmmgr_print_limine_memmap() {
    uint64_t num_memmap_entries                = kerndata.memmap.entry_count;
    struct limine_memmap_entry *memmap_entries = *kerndata.memmap.entries;
    for (size_t entry = 0; entry < num_memmap_entries; entry++) {
        uint64_t entry_type = memmap_entries[entry].type;
        if (entry_type == LIMINE_MEMMAP_KERNEL_AND_MODULES ||
            entry_type == LIMINE_MEMMAP_ACPI_NVS ||
            entry_type == LIMINE_MEMMAP_RESERVED) {
                printf("Mem Region %llu: ", entry_type);
                printf("Base: 0x%llx, ", memmap_entries[entry].base);
                printf("Length: 0x%llx, ", memmap_entries[entry].length);
                printf("Type: %u\r\n", memmap_entries[entry].type);
        }
    }
}
