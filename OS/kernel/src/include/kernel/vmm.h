#ifndef _VMM_H
#define _VMM_H
#include <kernel_ext/limine.h>

void vmmgr_init();
void vmmgr_map_everything(uint64_t pml4[]);
void vmmgr_map_kernel(uint64_t pml4[]);
void vmmgr_map_page(uint64_t pml4_addr[], uint64_t virtual_addr, uint64_t physical_addr, uint64_t num_pages, uint64_t flags);
void vmmgr_alloc_page(uint64_t pml4_addr[], uint64_t virtual_addr, uint64_t num_pages, uint64_t flags);

#endif
