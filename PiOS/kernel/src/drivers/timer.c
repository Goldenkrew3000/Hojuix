#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <drivers/gpio.h>
#include <drivers/uart.h>

#define SYSTMR_LO   ((volatile unsigned int*)(MMIO_BASE + 0x00003004))
#define SYSTMR_HI   ((volatile unsigned int*)(MMIO_BASE + 0x00003008))

// Delay for x CPU cycles
void delay_cycles(uint32_t cycles) {
    for (uint32_t i = 0; i < cycles; i++) {
        asm volatile("nop");
    }
}

// Delay for x microseconds (AArch64만)
void delay_arm_microsec(uint32_t msec) {
    register unsigned long freq;
    register unsigned long cntr;
    register unsigned long r;

    // Get the current counter frequency
    asm volatile("mrs %0, cntfrq_el0" : "=r"(freq));

    // Get the current counter
    asm volatile("mrs %0, cntpct_el0" : "=r"(cntr));

    // Calculate required count increase
    unsigned long incr = ((freq / 1000) * msec) / 1000;

    // Loop while counter increase is less than incr
    do {
        asm volatile("mrs %0, cntpct_el0" : "=r"(r));
    } while (r - cntr < incr); // TODO Convert to a for statement
}

// Delay for x milliseconds (AArch64U만)
void delay_arm_millisec(uint32_t msec) {
    delay_arm_microsec(msec * 1000);
}



// TODO: Fix and test the BCM timer
// Fetch the BCM system timer
uint64_t fetch_bcm_system_timer() {
    unsigned int h = -1;
    unsigned int l;

    // We must read the MMIO area as 2 separate 32-bit reads;
    h =* SYSTMR_HI;
    l =* SYSTMR_LO;
    
    // We have to repeat it if the high word changed during the read
    if (h !=* SYSTMR_HI) {
        h =* SYSTMR_HI;
        l =* SYSTMR_LO;
    }

    // Compose long int value (I am using uint64 here)
    return ((unsigned long) h << 32) | l;
}

// Delay x microseconds (BCM System Timer사용기)
void delay_bcm_microsec(uint32_t msec) {
    // Fetch the BCM system timer
    uint64_t bcm_timer = fetch_bcm_system_timer();

    // We MUST check if it's non-zero, because apparently QEMU does not emulate the BCM system timer. It would result in an infinite loop.
    if (bcm_timer) {
        while (fetch_bcm_system_timer() - bcm_timer < msec);
    } else {
        uart0_putstring("Kernel attempted to use the BCM System Timer on a Non-BCM SoC!\n");
    }
}
