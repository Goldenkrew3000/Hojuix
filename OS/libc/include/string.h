#ifndef _STRING_H
#define _STRING_H 1

#include <stddef.h>
#include <sys/cdefs.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void kern_memcpy(char* dest, const char* from, size_t n);
void* memmove(void*, const void*, size_t);
void *memset(void *s, int c, size_t n);
void kern_memset(uint8_t *array, uint8_t value, size_t size);
void kern_memset_word(uint8_t* array, uint32_t value, size_t size);
size_t strlen(const char*);

#endif
