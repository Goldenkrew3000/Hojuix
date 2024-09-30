#ifndef _PMMGR_H
#define _PMMGR_H
#include <stdint.h>
#include <kernel_ext/limine.h>

void pmmgr_init();
void pmmgr_init_single_bitmap(struct limine_memmap_entry memmap_entry);
uint64_t pmmgr_get_bitmap_reserved(struct limine_memmap_entry memmap_entry);
void* pmmgr_kmalloc(uint64_t num_pages);
bool pmmgr_check_pages_avaliable(uint64_t section_index, uint64_t page_frame_number, uint64_t num_pages, uint64_t bitmap_reserved);
void pmmgr_allocate_page(uint64_t section_index, uint64_t page_frame_number);
void pmmgr_allocate_pages(uint64_t section_index, uint64_t page_frame_number, size_t num_pages);

#endif 
