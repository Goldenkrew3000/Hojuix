#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <kern/kprintf.h>
#include <kern/libkern.h>
#include <kernel.h>
#include <arch/amd64/idt.h>
#include <memory/pmmgr.h>
#include <memory/vmmgr.h>
#include <arch/amd64/asm_functions.h>

extern void* isr_stub_table[];

#define IDT_FLAG_PRESENT      (1 << 7)
#define IDT_FLAG_RING3        (3 << 5)  // DPL=3 (user accessible)
#define IDT_FLAG_64BIT_INT    (0xE)     // 64-bit interrupt gate


void idt_init() {
    // Allocate a block of memory for the IDT
    //struct idt_entry_t *idt_content = (struct idt_entry_t*)((uint64_t)pmmgr_kmalloc(1) + ((uint64_t)kerndata.hhdm_offset));
    vmmgr_kalloc_page(0xa0000001000, 1); // Virtually map a page at 0xfffe000000000000
    uint64_t* idt_content_page = (uint64_t*)0xa0000001000; // Assign a uint64_t to 0xfffe000000000000
    i386_memset((void*)idt_content_page, 0x00, 4096); // Memset the page to 0x00
    struct idt_entry_t *idt_content = (struct idt_entry_t*)idt_content_page;

    // Assemble the IDT (TODO in this way, could use recursion to build this, but not right now)
    //idt_assemble_entry(0, (uint64_t)isr_stub_table[0], 0x8E, idt_content);
    for (int i = 0; i < 255; i++) {
        if (i <= 31) {
            idt_assemble_entry(i, (uint64_t)isr_stub_table[i], 0x8E, idt_content);
        }else {
            idt_assemble_entry(i, (uint64_t)isr_stub_table[i], IDT_FLAG_PRESENT | IDT_FLAG_RING3 | IDT_FLAG_64BIT_INT, idt_content);
        }

    }

    /*
    // TODO Keep these to deal with the stuff later
    idt_assemble_entry(1, &isr_debugException, 0x8E, idt_content);
    idt_assemble_entry(2, &isr_NMI, 0x8E, idt_content);
    idt_assemble_entry(3, &isr_breakpoint, 0x8E, idt_content);
    idt_assemble_entry(4, &isr_overflow, 0x8E, idt_content);
    idt_assemble_entry(5, &isr_boundaryRange, 0x8E, idt_content);
    idt_assemble_entry(6, &isr_undefinedOpcode, 0x8E, idt_content);
    idt_assemble_entry(7, &isr_deviceNotAvailable, 0x8E, idt_content);
    idt_assemble_entry(8, &isr_doubleFault, 0x8E, idt_content);
    idt_assemble_entry(10, &isr_invalidTSS, 0x8E, idt_content);
    idt_assemble_entry(11, &isr_notPresent, 0x8E, idt_content);
    idt_assemble_entry(12, &isr_stackSegment, 0x8E, idt_content);
    idt_assemble_entry(13, &isr_generalProtection, 0x8E, idt_content);
    idt_assemble_entry(14, &isr_pageFault, 0x8E, idt_content);
    */

    // Assemble the IDTR
    kerndata.idtr.size = (sizeof(struct idt_entry_t) * 256) - 1;
    kerndata.idtr.offset = (uint64_t)idt_content;

    // Actually utilize the newly created IDT
    asm("lidt %0" : : "m" (kerndata.idtr));
    printf("[IDT] Initialized.\n");
}

void idt_assemble_entry(uint32_t vector, uint64_t isr, uint8_t flags, struct idt_entry_t* idt_entry) {
    idt_entry[vector].offset1 = (uint64_t)isr;
    idt_entry[vector].offset2 = ((uint64_t)isr) >> 16;
    idt_entry[vector].offset3 = ((uint64_t)isr) >> 32;
    idt_entry[vector].flags = flags;
    idt_entry[vector].segmentSelector = 0x8;
}

void isr_handle(registers_t *r) {
    printf("there has been a major issue :/\n");
    printf("RDI is 0x%llx\n", r->rdi);
    printf("ISR Number: %d\n", r->isrNumber);

    printf("RAX: 0x%llx, RBX: 0x%llx, RCX: 0x%llx\n", r->rax, r->rbx, r->rcx);
    printf("Errno %d\n", r->errorCode);
    asm volatile("cli; hlt;");
}
