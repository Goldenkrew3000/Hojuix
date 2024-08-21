#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <kernel/io.h>
#include <kernel/dt.h>

// This code was found at http://www.osdever.net/tutorials/view/brans-kernel-development-tutorial
unsigned char inb(unsigned short _port) {
    unsigned char rv;
    __asm__ __volatile__("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void outb(unsigned short _port, unsigned char _data) {
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}