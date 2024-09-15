#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <drivers/gpio.h>
#include <drivers/uart.h>
#include <drivers/mbox.h>

// UART1 AUX Registers
#define AUX_ENABLE      ((volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO       ((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      ((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR      ((volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR      ((volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR      ((volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR      ((volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR      ((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL     ((volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT     ((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD     ((volatile unsigned int*)(MMIO_BASE+0x00215068))

// UART0 (PL011) Registers
#define UART0_DR        ((volatile unsigned int*)(MMIO_BASE+0x00201000))
#define UART0_FR        ((volatile unsigned int*)(MMIO_BASE+0x00201018))
#define UART0_IBRD      ((volatile unsigned int*)(MMIO_BASE+0x00201024))
#define UART0_FBRD      ((volatile unsigned int*)(MMIO_BASE+0x00201028))
#define UART0_LCRH      ((volatile unsigned int*)(MMIO_BASE+0x0020102C))
#define UART0_CR        ((volatile unsigned int*)(MMIO_BASE+0x00201030))
#define UART0_IMSC      ((volatile unsigned int*)(MMIO_BASE+0x00201038))
#define UART0_ICR       ((volatile unsigned int*)(MMIO_BASE+0x00201044))

void uart1_init() {
    register unsigned int r;

    *AUX_ENABLE |= 1; // Enable UART1
    *AUX_MU_CNTL = 0;
    *AUX_MU_LCR = 3; // 8 bits
    *AUX_MU_MCR = 0;
    *AUX_MU_IER = 0;
    *AUX_MU_IIR = 0xc6; // disable interrupts
    *AUX_MU_BAUD = 270; // 115200 baud

    // Map UART1 to GPIO
    r = *GPFSEL1;
    r &= ~((7 << 12) | (7 << 15)); // GPIO 14 and 15
    r |= (2 << 12) | (2 << 15); // ALT5
    *GPFSEL1 = r;
    *GPPUD = 0; // Enable pins 14 and 15
    r = 150;
    while (r--) {
        asm volatile("nop");
    }
    *GPPUDCLK0 = (1 << 14) | (1 << 15);
    r = 150;
    while (r--) {
        asm volatile("nop");
    }
    *GPPUDCLK0 = 0; // Flush GPIO Setup
    *AUX_MU_CNTL = 3; // Enable TX and RX
}

void uart1_putchar(char c) {
    do {
        asm volatile("nop");
    } while(!(*AUX_MU_LSR & 0x20));
    *AUX_MU_IO = c;
}

void uart1_putstring(char *string) {
    for (uint32_t i = 0; i < strlen(string); i++) {
        uart1_putchar(string[i]);
    }
}

void uart0_init() {
    register unsigned int r;

    // Initialize UART
    *UART0_CR = 0; // Turn off UART0
    
    // Setup clock for consistent divisor values
    mbox[0] = 9 * 4;
    mbox[1] = MBOX_REQUEST;
    mbox[2] = MBOX_TAG_SETCLKRATE; // Set the clock rate
    mbox[3] = 12;
    mbox[4] = 8;
    mbox[5] = 2; // UART Clock
    mbox[6] = 4000000; // 4 MHz
    mbox[7] = 0; // Clear Turbo
    mbox[8] = MBOX_TAG_LAST;
    mbox_call(MBOX_CH_PROP);

    // Map UART0 to GPIO Pins
    r = *GPFSEL1;
    r &= ~((7 << 12) | (7 << 15)); // GPIO 14 and 15
    r |= (4 << 12) | (4 << 15); // ALT0
    *GPFSEL1 = r;
    *GPPUD = 0; // Enable Pins 14 and 15
    r = 150;
    while (r--) {
        asm volatile("nop");
    }
    *GPPUDCLK0 = (1 << 14) | (1 << 15);
    r = 150;
    while (r--) {
        asm volatile("nop");
    }
    *GPPUDCLK0 = 0; // Flush GPIO Setup
    
    *UART0_ICR = 0x7FF; // Clear Interrupts
    *UART0_IBRD = 2; // 115200 bps
    *UART0_FBRD = 0xB;
    *UART0_LCRH = 0x7 << 4; // 8n1, enable FIFO
    *UART0_CR = 0x301; // Enable TX and RX;
}

void uart0_putchar(char c) {
    // Wait until we can send
    do {
        asm volatile("nop");
    } while (*UART0_FR & 0x20);

    // Write the character to the buffer
    *UART0_DR = c;
}

void uart0_putstring(char *string) {
    for (uint32_t i = 0; i < strlen(string); i++) {
        uart0_putchar(string[i]);
    }
}
