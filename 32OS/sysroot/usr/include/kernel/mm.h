#ifndef MM_H_INCLUDED
#define MM_H_INCLUDED
#include <stdint.h>
#include <kernel_ext/multiboot.h>

/*
// Multiboot Memory Map Handler
*/
extern uint32_t startkernel; // Defined in linker
extern uint32_t endkernel; // Defined in linker
void init_mmmh(multiboot_info_t* mbd, uint32_t magic);

/*
// PMM
*/

/*
typedef uint32_t physical_addr;
void mmap_set(int bit);
void mmap_unset(int bit);
bool mmap_test(int bit);
int mmap_first_free();
//int mmap_first_free_s(size_t size);
void pmmgr_init(uint32_t memSize, uint32_t bitmap_addr);
void pmmgr_init_region(physical_addr base, size_t size);
void pmmgr_deinit_region(physical_addr base, size_t size);
void* pmmgr_alloc_block();
//void* pmmgr_alloc_blocks(size_t size);
void pmmgr_free_block(void* p);
//void pmmgr_free_blocks(void* p, size_t size);
size_t pmmgr_get_memory_size();
uint32_t pmmgr_get_block_count();
uint32_t pmmgr_get_use_block_count();
uint32_t pmmgr_get_free_block_count();
uint32_t pmmgr_get_block_size();
*/

// DIY PMM
void pmmgr_init(uint32_t mem_size, uint32_t bitmap_addr);
uint32_t pmmgr_align(uint32_t addr);
uint32_t pmmgr_first_free();
void pmmgr_init_region(uint32_t addr, uint32_t size);

void pmmgr_set_bit(uint32_t addr, int bit);
void pmmgr_unset_bit(uint32_t addr, int bit);

/*
// VMM
*/

#endif