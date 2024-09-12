#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <drivers/gpio.h>
#include <drivers/uart.h>

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

void uart1_send(char c) {
    do {
        asm volatile("nop");
    } while(!(*AUX_MU_LSR & 0x01));
    *AUX_MU_IO = c;
}

void uart1_puts(char *string) {
    for (uint32_t i = 0; i < strlen(string); i++) {
        uart1_send(string[i]);
    }
}
