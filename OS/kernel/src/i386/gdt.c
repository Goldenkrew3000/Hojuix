#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kernel.h>
#include <i386/gdt.h>
#include <memory/pmmgr.h>
#include <memory/vmmgr.h>
#include <i386/asm_functions.h>

struct gdtr_t gdtr;
struct tss_t tss;

void gdt_init() {
    // Allocate a block from memory for the GDT / TSS
    //uint64_t* gdt_content = (uint64_t*)((uint64_t)pmmgr_kmalloc(1) + ((uint64_t)kerndata.hhdm_offset));

    vmmgr_kalloc_page(0xa0000000000, 1); // Virtually map a page at 0xfffe000000000000
    uint64_t* gdt_content = (uint64_t*)0xa0000000000; // Assign a uint64_t to 0xfffe000000000000
    i386_memset((void*)gdt_content, 0x00, 4096); // Memset the page to 0x00

    // Assemble the GDT
    //gdt_content[0] = gdt_assemble_entry(0, 0, 0, 0);
    //gdt_content[1] = gdt_assemble_entry(0, 0, 0x9A, 0x2);
    //gdt_content[2] = gdt_assemble_entry(0, 0, 0x92, 0);
    //gdt_content[3] = gdt_assemble_entry(0, 0, 0xFA, 0x2);
    //gdt_content[4] = gdt_assemble_entry(0, 0, 0xF2, 0);

    gdt_content[0] = create_gdt_entry(0, 0, 0, 0); // null
    gdt_content[1] = create_gdt_entry(0, 0, 0x9A, 0x2); // kernel code
    gdt_content[2] = create_gdt_entry(0, 0, 0x92, 0); // kernel data
    gdt_content[3] = create_gdt_entry(0, 0, 0xFA, 0x2); // user code
    gdt_content[4] = create_gdt_entry(0, 0, 0xF2, 0); // user data
    create_system_segment_descriptor(gdt_content, 5, (uint64_t)&tss, sizeof(struct tss_t) - 1, 0x89, 0);

    // Assemble the TSS
    //gdt_assemble_tss(gdt_content, 5, (uint64_t)&tss, sizeof(struct tss_t) - 1, 0x89, 0);

    // Assemble the GDTR
    gdtr.size = (sizeof(gdt_content[0]) * 7) - 1;
    gdtr.offset = (uint64_t)gdt_content;

    // Actually utilize the newly created structures
    /*
    asm("lgdt (%0)" : : "r" (&gdtr)); // Load the new GDT
    asm volatile("push $0x08; \
                  lea .gdt_farjmp(%%rip), %%rax; \
                  push %%rax; \
                  retfq; \
                  .gdt_farjmp: \
                  mov $0x10, %%eax; \
                  mov %%ax, %%ds; \
                  mov %%ax, %%es; \
                  mov %%ax, %%fs; \
                  mov %%ax, %%gs; \
                  mov %%ax, %%ss" : : : "eax", "rax"); // Far jump to the new GDT
    asm volatile("mov $0x28, %%ax; \
                  ltr %%ax" : : : "eax"); // Load the TSS
                  */
                  asm volatile(
                      "lgdt %0\n\t"
                      "push $0x08\n\t"      // Push new CS (kernel code segment)
                      "lea 1f(%%rip), %%rax\n\t"
                      "push %%rax\n\t"     // Push RIP for far return
                      "retfq\n\t"
                      "1:\n\t"
                      "mov $0x10, %%eax\n\t" // Reload data segments
                      "mov %%ax, %%ds\n\t"
                      "mov %%ax, %%es\n\t"
                      "mov %%ax, %%fs\n\t"
                      "mov %%ax, %%gs\n\t"
                      "mov %%ax, %%ss\n\t"
                      "ltr %%cx"           // Load TSS (CX must contain 16-bit selector)
                      :: "m"(gdtr), "c" (0x28) : "rax", "memory");
    printf("[GDT] Initialized.\n");
}

void tss_init() {
    tss.rsp0 = KERNEL_STACK_PTR;
    printf("[TSS] Initialized.\n");
}

/*
uint64_t gdt_assemble_entry(uint64_t base, uint64_t limit, uint64_t access, uint64_t flags) {
    // Assemble bases
    uint64_t base1 = base & 0xFFFF;
    uint64_t base2 = (base >> 16) & 0xFF;
    uint64_t base3 = (base >> 24) & 0xFF;

    // Assemble Limits
    uint64_t limit1 = limit & 0xFFFF;
    uint64_t limit2 = limit >> 16;

    // Actually assemble the GDT entry
    uint64_t gdt_entry = 0;
    gdt_entry |= limit1;
    gdt_entry |= limit2 << 48;
    gdt_entry |= base1 << 16;
    gdt_entry |= base2 << 32;
    gdt_entry |= base3 << 56;
    gdt_entry |= access << 40;
    gdt_entry |= flags << 52;

    return gdt_entry;
}

void gdt_assemble_tss(uint64_t* GDT, uint8_t index, uint64_t base, uint64_t limit, uint64_t access, uint64_t flags) {
    // Assemble Bases
    uint64_t base1 = base & 0xFFFF;
    uint64_t base2 = (base >> 16) & 0xFF;
    uint64_t base3 = (base >> 24) & 0xFF;
    uint64_t base4 = (base >> 32) & 0xFFFFFFFF;

    // Assemble Limits
    uint64_t limit1 = limit & 0xFFFF;
    uint64_t limit2 = (limit >> 16) & 0b1111;

    // Assemble the TSS
    GDT[index] = 0;
    GDT[index] |= limit1;
    GDT[index] |= base1 << 16;
    GDT[index] |= base2 << 32;
    GDT[index] |= access << 40;
    GDT[index] |= (limit2 & 0xF) << 48;
    GDT[index] |= (flags & 0xF) << 52;
    GDT[index] |= base3 << 56;
    GDT[index + 1] |= base4;
}
*/

uint64_t create_gdt_entry(uint64_t base, uint64_t limit, uint64_t access, uint64_t flags) {
    uint64_t base1  = base & 0xFFFF;
    uint64_t base2  = (base >> 16) & 0xFF;
    uint64_t base3  = (base >> 24) & 0xFF;
    uint64_t limit1 = limit & 0xFFFF;
    uint64_t limit2 = (limit >> 16) & 0b1111;
    uint64_t entry  = 0;
    entry |= limit1;
    entry |= limit2 << 48;
    entry |= base1  << 16;
    entry |= base2  << 32;
    entry |= base3  << 56;
    entry |= access << 40;
    entry |= flags  << 52;
    return entry;
}

void create_system_segment_descriptor(uint64_t *GDT, uint8_t idx, uint64_t base, uint64_t limit, uint64_t access, uint64_t flags) {
    uint64_t limit1 = limit & 0xFFFF;
    uint64_t limit2 = (limit >> 16) & 0b1111;
    uint64_t base1  = base & 0xFFFF;
    uint64_t base2  = (base >> 16) & 0xFF;
    uint64_t base3  = (base >> 24) & 0xFF;
    uint64_t base4  = (base >> 32) & 0xFFFFFFFF;
    GDT[idx] = 0;
    GDT[idx] |= limit1;
    GDT[idx] |= base1 << 16;
    GDT[idx] |= base2 << 32;
    GDT[idx] |= access << 40;
    GDT[idx] |= (limit2 & 0xF) << 48;
    GDT[idx] |= (flags & 0xF) << 52;
    GDT[idx] |= base3 << 56;
    GDT[idx + 1] = base4;
}
