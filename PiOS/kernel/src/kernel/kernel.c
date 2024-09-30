#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <drivers/uart.h>
#include <drivers/mbox.h>
#include <drivers/timer.h>
#include <drivers/framebuffer.h>
#include <memory/mmu.h>

#include <stdio.h>

void get_exception_level();
void get_board_serial_number();

#define KERNEL_UART0_DR        ((volatile unsigned int*)0xFFFFFFFFFFE00000)
#define KERNEL_UART0_FR        ((volatile unsigned int*)0xFFFFFFFFFFE00018)

void main() {
    // Initialize UART0
    uart0_init();

    // Initialize MMU
    mmu_init();

    // Initialize UART0
    uart0_putstring("this should NOT work\n");
    char *s = "wee paging\n";
    while(*s) {
        /* wait until we can send */
        do{asm volatile("nop");}while(*KERNEL_UART0_FR&0x20);
        /* write the character to the buffer */
        *KERNEL_UART0_DR=*s++;
    }

    //framebuffer_init();

    //get_exception_level();
    for(;;){}
}



void get_exception_level() {
    uint64_t el;
    asm volatile("mrs %0, CurrentEL" : "=r"(el));
    uart0_putstring("Current EL: ");
    uart0_hex((el>>2)&3);
    uart0_putstring("\n");
}

void get_board_serial_number() {
    mbox[0] = 8 * 4; // Length of the message
    mbox[1] = MBOX_REQUEST; // This is a request message
    
    mbox[2] = MBOX_TAG_GETSERIAL; // Get serial number command
    mbox[3] = 8; // Buffer size
    mbox[4] = 8;
    mbox[5] = 0; // Clear output buffer
    mbox[6] = 0;
    mbox[7] = MBOX_TAG_LAST;


    // Send the message to the GPU
    if (mbox_call(MBOX_CH_PROP)) {
        uart1_putstring("Got a response!\n");
    } else {
        uart1_putstring("Did not get a response...\n");
    }
}
