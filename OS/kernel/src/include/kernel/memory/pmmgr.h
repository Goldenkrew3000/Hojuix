#ifndef _PMMGR_H
#define _PMMGR_H

void pmmgr_init();
void pmmgr_set_used(uint64_t page);
void pmmgr_set_free(uint64_t page);
uint8_t pmmgr_is_page_used(uint64_t page);
uint64_t pmmgr_find_free_pages(uint64_t size);
void *pmmgr_kmalloc(uint64_t size);
void pmmgr_free(void *addr, uint64_t size);
void pmmgr_print_bitmap();

#endif
