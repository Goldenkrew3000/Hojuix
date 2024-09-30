#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <kernel/i386/io.h>
#include <kernel/i386/dt.h>
#include <kernel/drivers/ps2_keyboard.h>
#include <kernel/drivers/pit_timer.h>
#include <kernel/drivers/rs232.h>

/*
// GDT
*/

// MASSIVE Thank You to SpecOS for helping me with the GDT and IDT code!!
/*
struct GDTEntry gdt[5];

struct GDTEntry gdt_assembleEntry(uint8_t accessByte, uint8_t flags, uint32_t limit) {
    struct GDTEntry toReturn;
    toReturn.base1 = toReturn.base2 = toReturn.base3 = 0;
    toReturn.limit1 = limit & 0xFFFF;
    toReturn.limit2 = (limit >> 16) & 0xF;
    toReturn.accessByte = accessByte;
    toReturn.flags = flags;
    return toReturn;
}

void gdt_setGate(int gateID, uint8_t accessByte, uint8_t flags, uint32_t limit) {
    gdt[gateID] = gdt_assembleEntry(accessByte, flags, limit);
}









__attribute__((noinline))
void loadGDT() {
    printf("[GDT] Creating...\n");
    struct GDTPtr gdt_ptr;
    gdt_ptr.size = (sizeof(struct GDTEntry) * 5) - 1;
    gdt_ptr.offset = (uint64_t) &gdt;
    
    asm("lgdt (%0)" : : "r" (&gdt_ptr));
    //asm volatile("nop"); // General faults without keeping a little wait before continuing on
    asm volatile("push $0x08; \
                  lea .reload_CS(%%rip), %%rax; \
                  push %%rax; \
                  retfq; \
                  .reload_CS: \
                  mov $0x10, %%ax; \
                  mov %%ax, %%ds; \
                  mov %%ax, %%es; \
                  mov %%ax, %%fs; \
                  mov %%ax, %%gs; \
                  mov %%ax, %%ss" : : : "eax", "rax");
}

void gdt_init() {
    gdt_setGate(0, 0, 0, 0); // Null segment
    gdt_setGate(1, 0x9A, 0xA, 0xFFFFF); // Kernel Mode Code segment
    gdt_setGate(2, 0x92, 0xC, 0xFFFFF); // Kernel Mode Data segment
    gdt_setGate(3, 0xFA, 0xA, 0xFFFFF); // User Mode Code segment
    gdt_setGate(4, 0xF2, 0xC, 0xFFFFF); // User Mode Data segment
    // TODO TSS
    loadGDT();
}*/

/*
// IDT
*/

struct IDTEntry idt[256];

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

void idt_setDescriptor(uint8_t vect, void* isrThingy, uint8_t gateType, uint8_t dpl) {
    struct IDTEntry* thisEntry = &idt[vect];
    thisEntry->offset1 = (uint64_t)isrThingy & 0xFFFF;
    thisEntry->offset2 = ((uint64_t)isrThingy >> 16) & 0xFFFF;
    thisEntry->offset3 = ((uint64_t)isrThingy >> 32) & 0xFFFFFFFF;
    thisEntry->segmentSelector = 0x08;
    thisEntry->ist = 0;
    thisEntry->gateType = gateType;
    thisEntry->dpl = dpl;
    thisEntry->present = 1;
}

void idt_init() {
    // Setup the IDT pointer
    struct IDTPtr idt_ptr;
    idt_ptr.offset = (uintptr_t)&idt[0];
    idt_ptr.size = ((uint16_t)sizeof(struct IDTEntry) *  256) - 1;

    // Completely clear the IDT <-- Causes general pagefault
    //memset(&idt, 0, sizeof(struct idt_entry_s) * 256);

    // Setup Exception Handlers
    idt_setDescriptor(0x0, &isr_divideError, 15, 0);
    idt_setDescriptor(0x1, &isr_debugException, 15, 0);
    idt_setDescriptor(0x2, &isr_NMI, 15, 0);
    idt_setDescriptor(0x3, &isr_breakpoint, 15, 0);
    idt_setDescriptor(0x4, &isr_overflow, 15, 0);
    idt_setDescriptor(0x5, &isr_boundaryRange, 15, 0);
    idt_setDescriptor(0x6, &isr_undefinedOpcode, 15, 0);
    idt_setDescriptor(0x7, &isr_deviceNotAvailable, 15, 0);
    idt_setDescriptor(0x8, &isr_doubleFault, 15, 0);
    idt_setDescriptor(0xA, &isr_invalidTSS, 15, 0);
    idt_setDescriptor(0xB, &isr_notPresent, 15, 0);
    idt_setDescriptor(0xC, &isr_stackSegment, 15, 0);
    idt_setDescriptor(0xD, &isr_generalProtection, 15, 0);
    idt_setDescriptor(0xE, &isr_pageFault, 15, 0);
    // TODO: Add the rest of the exceptions

    // Load the IDT
    asm volatile("lidt %0" : : "m"(idt_ptr));
}

/*
// IRQ
*/

void irq_handler(int int_no) {
    int_no -= 32;

    // If the IDT entry that was invoked was greater than 40 (IRQ 8-15), send an EOI to the slave controller
    if (int_no >= 40) {
        out8(0xA0, 0x20);
    }

    // In either case, we need to send an EOI to the master interrupt controller
    out8(0x20, 0x20);
}

void irq_remap() {
    // Remap IRQ
    // Way more info at: https://github.com/jakeSteinburger/SpecOS/blob/main/sys/idt.c
    out8(0x20, 0x11);
    out8(0xA0, 0x11);
    out8(0x21, 0x20);
    out8(0xA1, 0x28);
    out8(0x21, 0x04);
    out8(0xA1, 0x02);
    out8(0x21, 0x01);
    out8(0xA1, 0x01);
    out8(0x21, 0xFF); // Mask interrupts --> 0xFF
    out8(0xA1, 0xFF); // Same here
}

void irq_init() {
    irq_remap();

    // NOTE: As per the OSDev Wiki, IRQ 0 is IDT 32

    // Set handler functions
    idt_setDescriptor(32, &irq_pit_timer_handler, 14, 0); // PIT Timer - IRQ 0
    idt_setDescriptor(33, &irq_keyboard_handler, 14, 0); // PS/2 Keyboard - IRQ 1
    idt_setDescriptor(35, &irq_rs232_port2_handler, 14, 0); // RS232 Port 2 - IRQ 3
    idt_setDescriptor(36, &irq_rs232_port1_handler, 14, 0); // RS232 Port 1 - IRQ 4

    uint8_t irq_mask;

    // Unmask the keyboard IRQ
    irq_mask ^= (1 << 1);

    // Unmask the PIT timer IRQ and set frequency
    irq_mask ^= (1 << 0);
    pit_timer_install();

    // Unmask RS232 Port 1 and 2
    irq_mask ^= (1 << 3);
    irq_mask ^= (1 << 4);

    //idt_setDescriptor(34, &irq_unknown_handler, 14, 0);
    //idt_setDescriptor(35, &irq_unknown_handler, 14, 0);
    //idt_setDescriptor(37, &irq_unknown_handler, 14, 0);
    //idt_setDescriptor(38, &irq_unknown_handler, 14, 0);
    //idt_setDescriptor(39, &irq_unknown_handler, 14, 0);

    // Unmask interrupts
    //outb(0x21, 0x00);

    // Send the IRQ Mask
    out8(0x21, irq_mask);
    
}

__attribute__((interrupt))
void irq_unknown_handler(void*) {
    printf("Received an unknown interrupt\n");
}
