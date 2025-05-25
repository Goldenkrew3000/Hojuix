#ifndef _KPRINTF_H
#define _KPRINTF_H
#include <stdarg.h>
#include <stddef.h>

// kern/kern_kprintf.c
void kprintf_putchar_fb(int c);

// kern/extern_printf.c
#define printf printf_
int printf_(const char* format, ...);
#define sprintf sprintf_
int sprintf_(char* buffer, const char* format, ...);
#define snprintf snprintf_
#define vsnprintf vsnprintf_
int snprintf_(char* buffer, size_t count, const char* format, ...);
int vsnprintf_(char* buffer, size_t count, const char* format, va_list va);
#define vprintf vprintf_
int vprintf_(const char* format, va_list va);
int fctprintf(void (*out)(char character, void* arg), void* arg, const char* format, ...);

#endif
