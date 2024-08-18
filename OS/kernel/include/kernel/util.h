#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <kernel_ext/multiboot.h>

void enable_cursor();
void move_cursor(uint32_t row, uint32_t column);
void reboot();
void multiboot_handler(multiboot_info_t* mbd, uint32_t magic);

#endif
