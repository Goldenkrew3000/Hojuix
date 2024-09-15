#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <drivers/uart.h>
#include <drivers/mbox.h>

#include <stdio.h>

void get_board_serial_number();

void main() {

    uart0_init();
    uart0_putstring("And this is an output at 115200bps on UART0!\n");
    uart0_putstring("And a 2nd output :3\n");
    printf("And this is from printf :)\n");
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
