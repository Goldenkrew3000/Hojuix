#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <kernel/dt.h>
#include <kernel/io.h>

long PIT_timer_ticks = 0;

void pit_timer_handler(struct regs *r) {
    // PIT Timer runs at 18.222Hz (By default)
    PIT_timer_ticks++;

    /*if (PIT_timer_ticks % 100 == 0) {
        printf("TIME %d\n", PIT_timer_ticks);
    }*/
}

void pit_timer_phase(int hz) {
    // Set what frequency the PIT timer runs at
    int divisor = 1193180 / hz;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);
}

void pit_timer_install() {
    // Set timer to 100hz
    pit_timer_phase(100);

    // Sets up the PIT System Clock on IRQ0
    irq_install_handler(0, pit_timer_handler);
}

void timer_wait(long milliseconds) {
    long ticks = milliseconds / 10;
    long wait_ticks = PIT_timer_ticks + ticks;
    while (PIT_timer_ticks < wait_ticks) {
        // O2 flags were compiling this wait out, so just make the processor do nothing instead
        asm volatile("nop");
    }
}