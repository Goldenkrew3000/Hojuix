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
// rs232_port_decider(). Returns 0 of port does not exist.
//
*/
int rs232_port_decider(int port) {
    // Convert port number to real port
    if (port == 1) {
        // COM1
        return 0x3F8;
    } else if (port == 2) {
        // COM2
        return 0x2F8;
    } else {
        // Selected serial port does not exist
        return 0;
    }
}

/*
// rs232_baudrate_decider().
// Takes a generic baudrate and returns the divisor.
// Returns 0 if the baudrate is not standard.
*/
int rs232_baudrate_decider(int baudrate) {
    if (baudrate == 38400) {
        //
    } else if (baudrate == 57600) {
        //
    } else if (baudrate == 115200) {
        //
    } else {
        // Baudrate is invalid
        return 0;
    }
}

/*
// rs232_init() --> Port: 1 --> 0x3F8, 2 --> 0x2F8, Baudrate: bps (115200). Returns 1 if port does not exist, Returns 2 if serial chip test failure
*/
int rs232_init(int port, long baudrate) {
    // Convert port number to real port
    int hex_port = rs232_port_decider(port);
    if (port == 0) {
        // Serial port does not exist
        return 1;
    }

    // Convert baudrate to divisor
    // NOTE: The UART controller has an internal clock of 115200 Hz
    // TODO

    // Actually initialize the serial port
    outb(hex_port + 1, 0x00); // Disable all interrupts
    outb(hex_port + 3, 0x80); // Enable DLAB
    outb(hex_port + 0, 0x02); // Set divisor to 2 (lo byte) 57600 baud
    outb(hex_port + 1, 0x00); // (hi byte)
    outb(hex_port + 3, 0x03); // 8 bits, no parity, 1 stop bit (8n1)
    outb(hex_port + 2, 0xC7); // Enable FIFO
    outb(hex_port + 4, 0x0B); // IRQs Enabled, RTS/DSR set
    
    // Test the serial chip
    outb(hex_port + 4, 0x1E); // Set in loopback mode (To test the serial chip)
    outb(hex_port + 0, 0xAE); // Send byte 0xAE for test
    if (inb(hex_port + 0) != 0xAE) {
        return 2;
    }

    // If serial chip is not faulty, set to normal operation mode
    outb(hex_port + 4, 0x0F);
    return 0;
}

int rs232_is_transmit_empty(int port) {
    // Convert port number to real port
    int hex_port = rs232_port_decider(port);
    if (port == 0) {
        // Serial port does not exist
        return 1;
    }

    return inb(hex_port + 5) & 0x20;
}

int rs232_write(int port, char character) {
    // Convert port number to real port
    int hex_port = rs232_port_decider(port);
    if (port == 0) {
        // Serial port does not exist
        return 1;
    }

    // Wait for the transmit buffer to be empty
    while (rs232_is_transmit_empty(1) == 0);

    // Write to serial port
    outb(hex_port, character);

    return 0;
}

__attribute__((interrupt))
void irq_rs232_port1_handler(void*) {
    // IRQ Handler for serial port 1 (0x3F8)
    printf("Data received on Serial Port 1\n");
}

__attribute__((interrupt))
void irq_rs232_port2_handler(void*) {
    // IRQ Handler for serial port 2 (0x2F8)
    printf("Data received on Serial Port 2\n");
}
