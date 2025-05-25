/*
// Hojuix PMMGR v3
*/

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <kern/kprintf.h>
#include <kern/libkern.h>
#include <kernel.h>
#include <memory/pmmgr.h>
#include <memory/vmmgr.h>
#include <arch/amd64/asm_functions.h>
#include <kernel_ext/limine.h>

// Limine Memory Map Variables
uint64_t total_usable_mem = 0;
uint64_t total_reserved_mem = 0;
uint64_t total_mem = 0;

// PMM Variables
uint64_t pmmgr_total_bitmap_pages = 0;
uint64_t pmmgr_used_bitmap_pages = 0;
uint64_t pmmgr_free_bitmap_pages = 0;
uint64_t pmmgr_bitmap_bytes = 0;
uint8_t* bitmap = (void*) 0;

void pmmgr_init() {
    // Fetch memory map info from limine
    uint64_t entry_count = kerndata.memmap.entry_count;
    struct limine_memmap_entry **entries = kerndata.memmap.entries;

    // Calculate the total usable and reserved memory
    for (uint64_t i = 0; i < entry_count; i++) {
        struct limine_memmap_entry *entry = entries[i];
        total_mem += entry->length;
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            total_usable_mem += entry->length;
        } else {
            total_reserved_mem += entry->length;
        }
    }

    // Calculate bitmap page count and size
    pmmgr_total_bitmap_pages = (total_mem + 0x1000 - 1) / 0x1000;
    pmmgr_bitmap_bytes = (pmmgr_total_bitmap_pages + 8 - 1) / 8; // Dividing by 8 because 8 pages per byte

    // Iterate over the memory map to find a spot large enough to store the bitmap
    for (uint64_t i = 0; i < entry_count; i++) {
        struct limine_memmap_entry *entry = entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            if (entry->length >= pmmgr_bitmap_bytes) {
                // Found a space large enough
                bitmap = (void*) (entry->base + 0xffff800000000000);
                break;
            }
        }
    }

    if (!bitmap) { // TODO Setup new way
        printf("Could not find a space large enough to store bitmap.");
        // TODO abort here
    }

    // Set the whole bitmap to used (All 1's / 0xFF)
    i386_memset(bitmap, 0xFF, pmmgr_bitmap_bytes);
    pmmgr_used_bitmap_pages = pmmgr_total_bitmap_pages;

    // Set the bitmap up according to the limine memory map (blacklist reserved memory etc) TODO CHECK
    for (uint64_t i = 0; i < entry_count; i++) {
        struct limine_memmap_entry *entry = entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            if (entry->base == (uint64_t)bitmap - 0xffff800000000000) {
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

    // TODO Add better way
    printf("Failed to find free memory.\n");
    // TODO abort here
}

// Returns the physical address to a page of memory
uintptr_t pmmgr_kmalloc(int size) {
    uint64_t free_page = pmmgr_find_free_pages(1);
    pmmgr_set_used(free_page);

    return ((uintptr_t)free_page) * 0x1000;
}

// Returns the physical address to x pages of contiguous memory
// size - Pages
uintptr_t pmmgr_kmalloc_contiguous(int size) {
    uint64_t needed_pages = size;
    uint64_t free_page = pmmgr_find_free_pages(size);

    //printf("Allocating free page\n");

    for (uint64_t i = 0; i < needed_pages; i++) {
        pmmgr_set_used(free_page + i);
    }

    return ((uintptr_t)free_page) * 0x1000;
}

// Frees a page - TODO FIX REMOVE SIZE
void pmmgr_kfree(uintptr_t physical_addr, uint64_t size) {
    uint64_t page = physical_addr / 0x1000;
    uint64_t pages = (size + 0x1000 - 1) / 0x1000;

    for (uint64_t i = 0; i < pages; i++) {
        pmmgr_set_free(page + i);
    }
}

void pmmgr_print_bitmap() {
    printf("[PMMGR] Total Pages: %lld, ", pmmgr_total_bitmap_pages);
    printf("Used Pages: %lld, ", pmmgr_used_bitmap_pages);
    printf("Free Pages: %lld\n", pmmgr_free_bitmap_pages);
}
