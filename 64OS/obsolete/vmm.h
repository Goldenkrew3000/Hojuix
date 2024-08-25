#ifndef _VMM_H
#define _VMM_H
#include <kernel_ext/limine.h>

typedef struct pagemap
{
    uint64_t * top_level;
} pagemap_t;

extern pagemap_t kernel_pagemap;

void vmmgr_init();
void vmmgr_map_page(uint64_t current_page_dir, uint64_t virtual_addr, uint64_t physical_addr, uint64_t flags);


#endif