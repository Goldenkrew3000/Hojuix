#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <kernel/i386/dt.h>
#include <kernel/i386/io.h>

long PIT_timer_ticks = 0;

__attribute__((interrupt))
void irq_pit_timer_handler(void*) {
    // PIT Timer runs at 18.222Hz (By default)
    PIT_timer_ticks++;

    /*if (PIT_timer_ticks % 100 == 0) {
        printf("TIME %d\n", PIT_timer_ticks);
    }*/

    // ACK the interrupt
    irq_handler(32);
}

void pit_timer_phase(int hz) {
    // Set what frequency the PIT timer runs at
    int divisor = 1193180 / hz;
    out8(0x43, 0x36);
    out8(0x40, divisor & 0xFF);
    out8(0x40, divisor >> 8);
}

void pit_timer_install() {
    // Set timer to 100hz
    pit_timer_phase(100);
}

void timer_wait(long milliseconds) {
    long ticks = milliseconds / 10;
    long wait_ticks = PIT_timer_ticks + ticks;
    while (PIT_timer_ticks < wait_ticks) {
        // O2 flags were compiling this wait out, so just make the processor do nothing instead
        asm volatile("nop");
    }
}
