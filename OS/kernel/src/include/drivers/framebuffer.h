#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H
#include <kernel_ext/limine.h>

void framebuffer_draw_pixel(int x, int y, uint32_t color);
void framebuffer_fill_background(uint32_t color);
void framebuffer_scroll(int lines);
void framebuffer_ssfn_init(struct limine_framebuffer *fb);
void framebuffer_putchar(char character);
void framebuffer_backspace();

#endif