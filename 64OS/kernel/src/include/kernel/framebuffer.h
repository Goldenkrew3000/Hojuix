#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H
#include <kernel_ext/limine.h>

void framebuffer_draw_pixel(struct limine_framebuffer *fb, int x, int y, uint32_t color);
void framebuffer_fill_background(struct limine_framebuffer *fb, uint32_t color);
void framebuffer_ssfn_init(struct limine_framebuffer *fb);
void framebuffer_putchar(char character);
void framebuffer_putstring(char* string);
size_t kern_strlen(char* string);

#endif