#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kernel.h>
#include <kernel/i386/gdt.h>
#include <kernel/memory/pmmgr.h>

struct gdtr_t gdtr;
struct tss_t tss;

void gdt_init() {
    printf("[GDT] Assembling...\n");

    // Allocate a block from memory for the GDT / TSS
    uint64_t* gdt_content = (uint64_t*)((uint64_t)pmmgr_kmalloc(1) + ((uint64_t)kerndata.hhdm_offset));

    // Assemble the GDT
    gdt_content[0] = gdt_assemble_entry(0, 0, 0, 0);
    gdt_content[1] = gdt_assemble_entry(0, 0, 0x9A, 0x2);
    gdt_content[2] = gdt_assemble_entry(0, 0, 0x92, 0);
    gdt_content[3] = gdt_assemble_entry(0, 0, 0xFA, 0x2);
    gdt_content[4] = gdt_assemble_entry(0, 0, 0xF2, 0);

    // Assemble the TSS
    gdt_assemble_tss(gdt_content, 5, (uint64_t)&tss, sizeof(struct tss_t) - 1, 0x89, 0);

    // Assemble the GDTR
    gdtr.size = (sizeof(gdt_content[0]) * 7) - 1;
    gdtr.offset = (uint64_t)gdt_content;

    // Actually utilize the newly created structures
    printf("[GDT] Far jumping...\n");
    asm("lgdt (%0)" : : "r" (&gdtr)); // Load the new GDT
    asm volatile("push $0x08; \
                  lea .gdt_farjmp(%%rip), %%rax; \
                  push %%rax; \
                  retfq; \
                  .gdt_farjmp: \
                  mov $0x10, %%ax; \
                  mov %%ax, %%ds; \
                  mov %%ax, %%es; \
                  mov %%ax, %%fs; \
                  mov %%ax, %%gs; \
                  mov %%ax, %%ss" : : : "eax", "rax"); // Far jump to the new GDT
    asm volatile("mov $0x28, %%ax; \
                  ltr %%ax" : : : "eax"); // Load the TSS
    printf("[GDT] Initialized.\n");
}

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
    
    printf("GDT current: %p\n", gdt_entry);

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
