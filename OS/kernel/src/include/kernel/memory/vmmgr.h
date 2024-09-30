#ifndef _VMMGR_H
#define _VMMGR_H
#include <kernel.h>
#include <kernel_ext/limine.h>

#define PAGE_ALIGN_DOWN(addr) ((addr / 4096) * 4096)
#define PAGE_ALIGN_UP(x) ((((x) + 4095) / 4096) * 4096)

#define TOPBITS 0xFFFF000000000000

#define KERNEL_PFLAG_PRESENT 0b1
#define KERNEL_PFLAG_WRITE   0b10
#define KERNEL_PFLAG_USER    0b100
#define KERNEL_PFLAG_PXD     0b10000000000000000000000000000000000000000000000000000000000000 // a bit long lmao

#define PAGE_SIZE 4096
#define KERNEL_STACK_PAGES 2LL
#define KERNEL_STACK_PTR 0xFFFFFFFFFFFFF000LL
#define KERNEL_STACK_ADDR KERNEL_STACK_PTR-(KERNEL_STACK_PAGES*PAGE_SIZE)

#define USER_STACK_PAGES 2LL
#define USER_STACK_ADDR (USER_STACK_PTR - USER_STACK_PAGES*PAGE_SIZE)
#define USER_STACK_PTR 0x700000000000LL

#define PAGE_ALIGN_DOWN(addr) ((addr / 4096) * 4096) // works cos of integer division
#define PAGE_ALIGN_UP(x) ((((x) + 4095) / 4096) * 4096)

#define KERNEL_SWITCH_PAGE_TREE(TREE_ADDRESS) \
    __asm__ volatile (\
       "movq %0, %%cr3"\
       :\
       :  "r" (TREE_ADDRESS)\
    )

#define KERNEL_SWITCH_STACK() \
    __asm__ volatile (\
       "movq %0, %%rsp\n"\
       "movq $0, %%rbp\n"\
       "push $0"\
       :\
       :  "r" (KERNEL_STACK_PTR)\
    )

#define vmmgr_switch_structures() \
    KERNEL_SWITCH_PAGE_TREE(kerndata.cr3); \
    KERNEL_SWITCH_STACK();

void vmmgr_init();
uint64_t vmmgr_virt_to_phys(uint64_t pml4_addr[], uint64_t virt_addr);
void vmmgr_write_vmem(uint64_t *pml4_addr, uint64_t virt_addr, char *data, size_t len);
void vmmgr_push_vmem(uint64_t *pml4_addr, uint64_t rsp, char *data, size_t len);
void vmmgr_map_pages(uint64_t pml4_addr[], uint64_t virt_addr, uint64_t phys_addr, uint64_t num_pages, uint64_t flags);
void vmmgr_alloc_pages(uint64_t pml4_addr[], uint64_t virt_addr, uint64_t num_pages, uint64_t flags);
void vmmgr_map_sections(uint64_t pml4[]);
void vmmgr_map_kernel(uint64_t pml4[]);
void vmmgr_map_all(uint64_t pml4[]);

#endif
