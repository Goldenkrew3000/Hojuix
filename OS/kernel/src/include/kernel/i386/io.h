#ifndef _IO_H
#define _IO_H
#include <kernel/i386/dt.h>

unsigned char in8(unsigned short _port);
void out8(unsigned short _port, unsigned char _data);
uint16_t in16(unsigned short _port);
void out16(unsigned short _port, uint16_t _data);
uint32_t in32(unsigned short _port);
void out32(unsigned short _port, uint32_t _data);

#endif
