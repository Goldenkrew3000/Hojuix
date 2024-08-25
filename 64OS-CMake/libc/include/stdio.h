#ifndef _STDIO_H
#define _STDIO_H 1

#include <stdarg.h>
#include <stddef.h>
#include <sys/cdefs.h>

#define EOF (-1)

int putchar(int);
int puts(const char*);

// Printf implementation used is from https://github.com/mpaland/printf
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
