#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kernel/i386/irq.h>
#include <kernel/i386/io.h>

void irq_init() {
    // Configure and mask all interrupts
    irq_configure();

    printf("[IRQ] Initialized.\n");
}

void irq_configure() {
    out8(0x20, 0x11);
    out8(0xA0, 0x11);
    out8(0x21, 0x20);
    out8(0xA1, 0x28);
    out8(0x21, 0x04);
    out8(0xA1, 0x02);
    out8(0x21, 0x01);
    out8(0xA1, 0x01);
    out8(0x21, 0xFF);
    out8(0xA1, 0xFF);
}

void irq_mask(int irq_number) {
    if (irq_number < 8) {
        out8(0x21, (1 << (irq_number % 8)));
    } else {
        out8(0xA1, (1 << (irq_number % 8)));
    }
}

void irq_unmask(int irq_number) {
    if (irq_number < 8) {
        out8(0x21, ~(1 << (irq_number % 8)));
    } else {
        out8(0xA1, ~(1 << (irq_number % 8)));
    }
}

void irq_ack(int irq_number) {
    irq_number -= 32;

    // If the IRQ that was invoked was greater than 8 (IRQ 8-15), send an EOI to the slave controller
    if (irq_number >= 8) {
        out8(0xA0, 0x20);
    }
    
    // In either case, send an EOI to the master interrupt controller
    out8(0x20, 0x20);
}
