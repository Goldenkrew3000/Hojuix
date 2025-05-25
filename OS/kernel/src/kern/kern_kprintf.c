#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <kern/kprintf.h>
#include <drivers/framebuffer.h>

void kprintf_init() {
    //
}

void kprintf_putchar_fb(int c) {
    framebuffer_putchar((char)c);
}
