#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kernel.h>
#include <kernel/i386/idt.h>
#include <kernel/memory/pmmgr.h>
#include <kernel/memory/vmmgr.h>

void idt_init() {
    // Allocate a block of memory for the IDT
    struct idt_entry_t *idt_content = (struct idt_entry_t*)((uint64_t)pmmgr_kmalloc(1) + ((uint64_t)kerndata.hhdm_offset));
    
    // Assemble the IDT
    idt_assemble_entry(0, &isr_divideError, 0x8E, idt_content);
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

    // Assemble the IDTR
    kerndata.idtr.size = (sizeof(struct idt_entry_t) * 256) - 1;
    kerndata.idtr.offset = (uint64_t)idt_content;

    // Actually utilize the newly created IDT
    asm("lidt %0" : : "m" (kerndata.idtr));
    printf("[IDT] Initialized.\n");
}

void idt_assemble_entry(uint32_t vector, void *isr, uint8_t flags, struct idt_entry_t* idt_entry) {
    idt_entry[vector].offset1 = (uint64_t)isr;
    idt_entry[vector].offset2 = ((uint64_t)isr) >> 16;
    idt_entry[vector].offset3 = ((uint64_t)isr) >> 32;
    idt_entry[vector].flags = flags;
    idt_entry[vector].segmentSelector = 0x8;
}

// Exception 0x0
__attribute__((interrupt))
void isr_divideError(void*) {
    printf("ISR: Divide Exception (0x0)\n");
    abort();
}

// Exception 0x1
__attribute__((interrupt))
void isr_debugException(void*) {
    printf("ISR: Debug Exception (0x1)\n");
    abort();
}

// Exception 0x2
__attribute__((interrupt))
void isr_NMI(void*) {
    printf("ISR: NMI (0x2)\n");
    abort();
}

// Exception 0x3
__attribute__((interrupt))
void isr_breakpoint(void*) {
    printf("ISR: Breakpoint (0x3)\n");
    abort();
}

// Exception 0x4
__attribute__((interrupt))
void isr_overflow(void*) {
    printf("ISR: Overflow (0x4)\n");
    abort();
}

// Exception 0x5
__attribute__((interrupt))
void isr_boundaryRange(void*) {
    printf("ISR: Boundary range exceeded (0x5)\n");
    abort();
}

// Exception 0x6
__attribute__((interrupt))
void isr_undefinedOpcode(void*) {
    printf("ISR: Undefined Opcode (0x6)\n");
    abort();
}

// Exception 0x7
__attribute__((interrupt))
void isr_deviceNotAvailable(void*) {
    printf("ISR: Device not available (0x7)\n");
    abort();
}

// Exception 0x8
__attribute__((interrupt))
void isr_doubleFault(void*) {
    printf("ISR: Double Fault (0x8)\n");
    abort();
}

// Exception 0x9 is reserved (옛날에 80387 Coprocessor Segment Overrun)
// Exception 0xA
__attribute__((interrupt))
void isr_invalidTSS(void*) {
    printf("ISR: Invalid TSS (0xA)\n");
    abort();
}

// Exception 0xB
__attribute__((interrupt))
void isr_notPresent(void*) {
    printf("ISR: Not Present (0xB)\n");
    abort();
}

// Exception 0xC
__attribute__((interrupt))
void isr_stackSegment(void*) {
    printf("ISR: Stack Segment (0xC)\n");
    abort();
}

// Exception 0xD
__attribute__((interrupt))
void isr_generalProtection(void*) {
    printf("ISR: General Protection (0xD)\n");
    abort();
}

// Exception 0xE
__attribute__((interrupt))
void isr_pageFault(void*) {
    printf("ISR: Page Fault (0xE)\n");
    abort();
}

