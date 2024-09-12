#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kernel/pmm.h>
#include <kernel_ext/limine.h>

// Created by loosely following the tutorial from: https://github.com/Menotdan/OSdev-tutorial/blob/master/chapters/4/chapter.md
// NOTE: The tutorial sets every for loop as i = 1, not sure why considering the memory is marked available but if I run into issues, I did make that change
// NOTE: Not sure if I should be using virtual or physical address for stuff here, so most stuff is using the virtual address

// Request to get the memory map
__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

// Limine Memory Map Variables
uint64_t total_usable_mem = 0;
uint64_t total_reserved_mem = 0;
uint64_t total_mem = 0;
uint64_t* hhdm_offset = (uint64_t*)0xFFFF800000000000; // Hardcoded due to it always being the same

// PMM Variables
uint64_t pmmgr_total_bitmap_pages = 0;
uint64_t pmmgr_used_bitmap_pages = 0;
uint64_t pmmgr_free_bitmap_pages = 0;
uint64_t pmmgr_bitmap_bytes = 0;
uint8_t* bitmap = (void*) 0;

void pmmgr_init() { // Feed mem_size ALL (All types) memory, in bytes, and bitmap_addr 0x20C000
    // Check we have a memory map response
    if (memmap_request.response == NULL) {
        printf("Limine Memory Map response does not exist.\n");
        abort();
    }

    // Fetch memory map info from limine
    uint64_t entry_count = memmap_request.response->entry_count;
    struct limine_memmap_entry **entries = memmap_request.response->entries;
    for (uint64_t i = 0; i < entry_count; i++) {
        struct limine_memmap_entry *entry = entries[i];
        //printf("Mem Region %llu: ", i);
        //printf("Base: 0x%lx, ", entry->base);
        //printf("Length: 0x%lx, ", entry->length);
        //printf("Type: %u\n", entry->type);

        total_mem += entry->length;
        if (entry->type == LIMINE_MEMMAP_USABLE || entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            total_usable_mem += entry->length;
        } else {
            total_reserved_mem += entry->length;
        }
    }

    // Print basic memory info to console
    printf("Total Memory: %lld mb (", total_mem / 1024 / 1024);
    printf("%lld mb usable / ", total_usable_mem / 1024 / 1024);
    printf("%lld mb reserved)\n", total_reserved_mem / 1024 / 1024);
    //printf("HHDM offset: %lx\n", hhdm_offset);
    
    // Calculate bitmap page count and size
    pmmgr_total_bitmap_pages = (total_mem + 0x1000 - 1) / 0x1000;
    pmmgr_bitmap_bytes = (pmmgr_total_bitmap_pages + 8 - 1) / 8; // Dividing by 8 because 8 pages per byte

    // Iterate over the memory map to find a spot large enough to store the bitmap
    for (uint64_t i = 0; i < entry_count; i++) {
        struct limine_memmap_entry *entry = entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE || entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            if (entry->length >= pmmgr_bitmap_bytes) {
                // Found a space large enough
                //printf("Bitmap Location: 0x%lx - ", entry->base);
                //printf("0x%lx\n", entry->base + pmmgr_bitmap_bytes);
                bitmap = (void*) (entry->base + (uint64_t)hhdm_offset);
                break;
            }
        }
    }

    if (!bitmap) {
        printf("Could not find a space large enough to store bitmap.");
        abort();
    }

    // Set the whole bitmap to used (All 1's / 0xFF)
    memset(bitmap, 0xFF, pmmgr_bitmap_bytes);
    pmmgr_used_bitmap_pages = pmmgr_total_bitmap_pages;

    // Set the bitmap up according to the limine memory map (blacklist reserved memory etc) TODO CHECK
    for (uint64_t i = 0; i < entry_count; i++) {
        struct limine_memmap_entry *entry = entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE || entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            
            if (entry->base == (uint64_t)bitmap - (uint64_t)hhdm_offset) {
                // This is where the bitmap is stored, do not want to mark the bitmap as free memory
                uint64_t bitmap_end_byte = (uint64_t)bitmap + pmmgr_bitmap_bytes;
                uint64_t bitmap_end_page = ((bitmap_end_byte + 0x1000 - 1) / 0x1000) * 0x1000;
                uint64_t entry_end_page = (entry->base + entry->length) / 0x1000; // The tutorial says usable pages are guaranteed to be page aligned in stivale. Usable and bootloader reclaimable are also in limine.

                // Continue until we have freed all pages
                for (uint64_t page = bitmap_end_page; page < entry_end_page; page++) {
                    pmmgr_set_free(page);
                }
            } else {
                uint64_t page = entry->base / 0x1000;
                uint64_t count = entry->length / 0x1000;

                for (uint64_t j = 0; j < count; j++) {
                    pmmgr_set_free(page + j);
                }
            }
        }
    }
}

void pmmgr_set_used(uint64_t page) {
    uint64_t byte = page / 8;
    uint64_t bit = page % 8;
    bitmap[byte] |= (1 << bit);
    pmmgr_used_bitmap_pages++;
    pmmgr_free_bitmap_pages--;
}

void pmmgr_set_free(uint64_t page) {
    uint64_t byte = page / 8;
    uint64_t bit = page % 8;
    bitmap[byte] &= ~(1 << bit);
    pmmgr_used_bitmap_pages--;
    pmmgr_free_bitmap_pages++;
}

uint8_t pmmgr_is_page_used(uint64_t page) {
    uint64_t byte = page / 8;
    uint64_t bit = page % 8;
    return (bitmap[byte] & (1 << bit)) >> bit;
}

uint64_t pmmgr_find_free_pages(uint64_t size) {
    uint64_t needed_pages = (size + 0x1000 - 1) / 0x1000;
    uint64_t found_pages = 0;
    uint64_t current_page = 0;

    for (uint64_t i = 0; i < pmmgr_total_bitmap_pages; i++) {
        if (!pmmgr_is_page_used(i)) {
            if (found_pages == 0) {
                current_page = i;
            }
            found_pages++;
        } else {
            found_pages = 0;
        }

        if (found_pages >= needed_pages) {
            return current_page;
        }
    }

    printf("Failed to find free memory.\n");
    abort();
}

void *pmmgr_malloc(uint64_t size) { // Size in bytes
    uint64_t needed_pages = (size + 0x1000 - 1) / 0x1000;
    uint64_t free_page = pmmgr_find_free_pages(size);

    //printf("Allocating free page\n");

    for (uint64_t i = 0; i < needed_pages; i++) {
        pmmgr_set_used(free_page + i);
    }

    return (void*)(free_page * 0x1000); // Returns physical addr
}

void pmmgr_free(void *addr, uint64_t size) { // 현재 몰라...
    uint64_t page = (uint64_t)addr / 0x1000;
    uint64_t pages = (size + 0x1000 - 1) / 0x1000;

    for (uint64_t i = 0; i < pages; i++) {
        pmmgr_set_free(page + i);
    }
}

void pmmgr_print_bitmap() {
    printf("Total Bitmap Pages: %lld, ", pmmgr_total_bitmap_pages);
    printf("Used Bitmap Pages: %lld, ", pmmgr_used_bitmap_pages);
    printf("Free Bitmap Pages: %lld\n", pmmgr_free_bitmap_pages);
}

