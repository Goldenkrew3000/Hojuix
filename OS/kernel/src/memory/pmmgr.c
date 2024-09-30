#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kernel.h>
#include <kernel/memory/pmmgr.h>
#include <kernel_ext/limine.h>

/*
 * Tbh I have mostly no idea what is going on here.
 * Most of this code is from PotatOS.
 * I have been stuck on non-functional paging for over a month.
 * It works, and that's all that matters.
 * Why is memory management / paging so fucking fragile...
 */

void pmmgr_init() {
    printf("[PMMGR] Initializing...\n");

    // Fetch Limine Memory Map
    struct limine_memmap_entry *memmap_entries = *kerndata.memmap.entries;
    uint64_t memmap_entry_count = kerndata.memmap.entry_count;

    // Initialize Bitmap
    for (size_t i = 0; i < memmap_entry_count; i++) {
        pmmgr_init_single_bitmap(memmap_entries[i]);
    }

    kerndata.last_freed_section = -1;
    kerndata.last_freed_page = -1;
}

void pmmgr_init_single_bitmap(struct limine_memmap_entry memmap_entry) {
    // Check whether the current memory map entry is usable
    if (memmap_entry.type != LIMINE_MEMMAP_USABLE) {
        return;
    }

    // All memory is usable at this point, completely zero it out
    kern_memset((uint8_t*)memmap_entry.base + kerndata.hhdm_offset, 0, memmap_entry.length);
    //kern_memset_word((uint8_t*)memmap_entry.base + kerndata.hhdm_offset, 0xABCDABCD, memmap_entry.length);
}

uint64_t pmmgr_get_bitmap_reserved(struct limine_memmap_entry memmap_entry) {
    uint64_t bitmap_reserved = 0;
    uint32_t n = 1;
    while (true) {
        if (n * 4096 * 8 > (memmap_entry.length / 4096) - n) {
            bitmap_reserved = n * 4096;
            break;
        }
    }
    return bitmap_reserved;
}

void* pmmgr_kmalloc(uint64_t num_pages) {
    if (num_pages < 1) {
        printf("[PMMGR] Cannot allocate less than 1 page! Halting.\n");
        abort();
    }

    struct limine_memmap_entry *memmap_entries = *kerndata.memmap.entries;
    if (kerndata.last_freed_page != -1 && num_pages < kerndata.last_freed_num_pages) {
        pmmgr_allocate_pages(kerndata.last_freed_section, kerndata.last_freed_page, num_pages);
        uint64_t bitmap_reserved = pmmgr_get_bitmap_reserved(memmap_entries[kerndata.last_freed_section]);
        return (void*) (memmap_entries[kerndata.last_freed_section].base + (kerndata.last_freed_page * 4096) + bitmap_reserved);
    }

    for (size_t entry = 0; entry < kerndata.memmap.entry_count; entry++) {
        // look through this entry for avaliable pages
        if (memmap_entries[entry].type != LIMINE_MEMMAP_USABLE) continue;
        uint64_t bitmap_reserved = pmmgr_get_bitmap_reserved(memmap_entries[entry]);
        uint64_t max_bitmap_bytes = ((memmap_entries[entry].length - bitmap_reserved) / 4096) / 8;
        for (size_t bitmap_byte = 0; bitmap_byte < max_bitmap_bytes; bitmap_byte++) {
            for (uint8_t bitmap_bit = 0; bitmap_bit < 8; bitmap_bit++) {
                if (pmmgr_check_pages_avaliable(entry, bitmap_bit + (bitmap_byte * 8), num_pages, bitmap_reserved)) {
                    pmmgr_allocate_pages(entry, bitmap_bit + (bitmap_byte * 8), num_pages);
                    uintptr_t addr = (memmap_entries[entry].base + ((bitmap_bit + (bitmap_byte * 8)) * 4096) + bitmap_reserved);
                    kern_memset((uint8_t*) (addr + kerndata.hhdm_offset), 0, num_pages * 4096);
                    return (void*) addr;
                }
            }
        }
    }

    // No physical memory left, halt
    printf("[PMMGR] No more physical memory available! Halting.\n");
    abort();
}

bool pmmgr_check_pages_avaliable(uint64_t section_index, uint64_t page_frame_number, uint64_t num_pages, uint64_t bitmap_reserved) {
    struct limine_memmap_entry *memmap_entries = *kerndata.memmap.entries;
    uint64_t max_bitmap_bytes = ((memmap_entries[section_index].length - bitmap_reserved) / 4096) / 8;
    uint64_t pages_in_section = max_bitmap_bytes * 8;
    uint64_t top_page = page_frame_number + num_pages;
    if (top_page > pages_in_section) {
        return false;
    }
    uint8_t *bitmap_start = (uint8_t*) (*kerndata.memmap.entries)[section_index].base + kerndata.hhdm_offset;
    if (memmap_entries[section_index].type != LIMINE_MEMMAP_USABLE) {
        return false;
    }
    if (pages_in_section < num_pages) {
        return false;
    }
    for (; page_frame_number < top_page; page_frame_number++) {
        // check if the single page is avaliable. If not, break.
        uint64_t byte = page_frame_number / 8;
        uint64_t bit  = page_frame_number % 8;
        if ((bitmap_start[byte] >> bit) & 1) return false;
    }

    // No pages were marked as used.
    return true;
}

void pmmgr_allocate_page(uint64_t section_index, uint64_t page_frame_number) {
    uint8_t* bitmap_start = (uint8_t*) (*kerndata.memmap.entries)[section_index].base + kerndata.hhdm_offset;
    uint64_t byte = page_frame_number / 8;
    uint64_t bit = page_frame_number % 8;
    uint8_t or_value = (bit) ? (1 << bit) : 1;
    uint8_t* current_byte = (uint8_t*) (((uint64_t) bitmap_start) + (uint64_t)(byte));
    *current_byte = *current_byte | or_value;
}

void pmmgr_allocate_pages(uint64_t section_index, uint64_t page_frame_number, size_t num_pages) {
    for (size_t i = 0; i < num_pages; i++) {
        pmmgr_allocate_page(section_index, page_frame_number + i);
    }
}
