#ifndef _LIBKERNEL_H
#define _LIBKERNEL_H
#include <stddef.h>

int kern_abs(int a);
#define abs(a) kern_abs(a)

void* kern_memcpy(void *, const void *, size_t);
int kern_memcmp(const void *, const void *, size_t);
void* kern_memset(void *, int, size_t);
#define memcpy(d, s, l) kern_memcpy(d, s, l)
#define memcmp(a, b, l) kern_memcmp(a, b, l)
#define memset(d, v, l) kern_memset(d, v, l)

size_t kern_strlen(const char *);
#define	strlen(a) kern_strlen(a)

#endif
