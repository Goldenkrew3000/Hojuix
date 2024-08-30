#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <kernel/io.h>

/*
// RS232 Port Addresses
// COM1 - 0x3F8
// COM2 - 0x2F8
// COM3 - COM8 not supported as of now
*/

/*
// rs232_init() --> Port: 1 --> 0x3F8, 2 --> 0x2F8, Baudrate: bps (115200). Returns 1 if port does not exist
*/

int rs232_init(int port, long baudrate) {
    // Convert port number to real port
    int hex_port = 0;

    if (port == 1) {
        // COM1
        hex_port = 0x3F8;
    } else if (port == 2) {
        // COM2
        hex_port = 0x2F8;
    } else {
        // Selected serial port does not exist
        printf("RS232 Port %d does not exist.\n", port);
    }

    // Convert baudrate to 
}
