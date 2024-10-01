#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <kernel.h>
#include <kernel/i386/idt.h>
#include <kernel/i386/irq.h>
#include <kernel/i386/io.h>
#include <kernel/drivers/pit_timer.h>

long PIT_timer_ticks = 0;

void pit_timer_init() {
    // Set the ISR Interrupt Handler Function
    idt_assemble_entry(32, &irq_pit_timer_handler, 0x8E, (struct idt_entry_t*)kerndata.idtr.offset);

    // Unmask the PIT Timer IRQ (IRQ 0)
    irq_unmask(0);

    // Set timer to 100hz
    pit_timer_phase(100);
}

__attribute__((interrupt))
void irq_pit_timer_handler(void*) {
    // PIT Timer runs at 18.222Hz (By default)
    PIT_timer_ticks++;

    if (PIT_timer_ticks % 100 == 0) {
        printf("TIME %d\n", PIT_timer_ticks);
    }

    // ACK the interrupt
    irq_ack(0);
}

void pit_timer_phase(int hz) {
    // Set what frequency the PIT timer runs at
    int divisor = 1193180 / hz;
    out8(0x43, 0x36);
    out8(0x40, divisor & 0xFF);
    out8(0x40, divisor >> 8);
}

void timer_wait(long milliseconds) {
    long ticks = milliseconds / 10;
    long wait_ticks = PIT_timer_ticks + ticks;
    while (PIT_timer_ticks < wait_ticks) {
        // O2 flags were compiling this wait out, so just make the processor do nothing instead
        asm volatile("nop");
    }
}
