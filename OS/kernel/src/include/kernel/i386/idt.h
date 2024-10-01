#ifndef _IDT_H
#define _IDT_H

struct idtr_t {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed));

struct idt_entry_t {
    uint16_t offset1;
    uint16_t segmentSelector;
    uint8_t rsvd;
    uint8_t flags;
    uint16_t offset2;
    uint32_t offset3;
    uint32_t reserved;
} __attribute__((packed));

void idt_init();
void idt_assemble_entry(uint32_t vector, void *isr, uint8_t flags, struct idt_entry_t* idt_entry);
void isr_divideError(void*);
void isr_debugException(void*);
void isr_NMI(void*);
void isr_breakpoint(void*);
void isr_overflow(void*);
void isr_boundaryRange(void*);
void isr_undefinedOpcode(void*);
void isr_deviceNotAvailable(void*);
void isr_doubleFault(void*);
void isr_invalidTSS(void*);
void isr_notPresent(void*);
void isr_stackSegment(void*);
void isr_generalProtection(void*);
void isr_pageFault(void*);

#endif
