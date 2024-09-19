#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <kernel/io.h>
#include <kernel/dt.h>

unsigned char in8(unsigned short _port) {
    unsigned char rv;
    asm volatile("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void out8(unsigned short _port, unsigned char _data) {
    asm volatile ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

uint16_t in16(unsigned short _port) {
    uint16_t ret;
    asm volatile("inw %w1, %w0" : "=a" (ret) : "Nd" (_port));
    return ret;
}

void out16(unsigned short _port, uint16_t _data) {
    asm volatile("outw %w0, %w1" : : "a" (_data), "Nd" (_port));
}

uint32_t in32(unsigned short _port) {
    uint32_t ret;
    asm volatile("inl %w1, %0" : "=a" (ret) : "Nd" (_port));
    return ret;
}

void out32(unsigned short _port, uint32_t _data) {
    asm volatile("outl %0, %w1" : : "a" (_data), "Nd" (_port));
}
