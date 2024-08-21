#ifndef TTY_H_INCLUDED
#define TTY_H_INCLUDED

#include <stdint.h>
#include <stddef.h>

void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
void terminal_putentryat(char character, uint8_t color, size_t x, size_t y);
void terminal_putchar(char character);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_scroll();

#endif