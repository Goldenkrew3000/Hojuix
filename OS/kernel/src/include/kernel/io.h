#ifndef _IO_H
#define _IO_H
#include <kernel/dt.h>

unsigned char inb(unsigned short _port);
void outb(unsigned short _port, unsigned char _data);

#endif