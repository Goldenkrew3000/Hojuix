#ifndef IO_H_INCLUDED
#define IO_H_INCLUDED

#include <kernel/dt.h>

unsigned char inb(unsigned short _port);
void outb(unsigned short _port, unsigned char _data);

#endif
