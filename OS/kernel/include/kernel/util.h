#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <kernel_ext/multiboot.h>

void enable_cursor();
void move_cursor(int row, int column);
void multiboot_handler(multiboot_info_t* mbd, uint32_t magic);

#endif
