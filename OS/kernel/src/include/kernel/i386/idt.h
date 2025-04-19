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

typedef struct {
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t rbp;
	uint64_t rdx;
	uint64_t rcx;
	uint64_t rbx;
	uint64_t rax;
	//uint64_t core;
	uint64_t isrNumber;
	uint64_t errorCode;
	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss;
} __attribute__((packed)) registers_t;

void idt_init();
void idt_assemble_entry(uint32_t vector, uint64_t isr, uint8_t flags, struct idt_entry_t* idt_entry);
void isr_handle(registers_t *r);
/*
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
*/

#endif
