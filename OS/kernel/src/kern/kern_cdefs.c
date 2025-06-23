#include <kern/libkern.h>
#include <arch/amd64/asm_functions.h>

int kern_abs(int a)
{
    return a>0 ? a : -a;
}

void* kern_memcpy(void *dst, const void *src, size_t len)
{
    i386_memcpy(dst, src, len);
}

int kern_memcmp(const void *b1, const void *b2, size_t len)
{
    const unsigned char* a = (const unsigned char*) b1;
	const unsigned char* b = (const unsigned char*) b2;
	for (size_t i = 0; i < len; i++) {
		if (a[i] < b[i])
			return -1;
		else if (b[i] < a[i])
			return 1;
	}
	return 0;
}

void* kern_memset(void *b, int c, size_t len)
{
    i386_memset(b, c, len);
}

void* kern_memmove(void *dst, const void *src, size_t len)
{
    return __builtin_memmove(dst, src, len);
}

size_t kern_strlen(const char *str)
{
    const char *s;

    s = str;
    while (1) {
        if (*s == '\0')
            break;
        s++;
    }

    return (s - str);
}

// XNU 11417.121.6
int kern_strcmp(const char* s1, const char* s2) {
    while (*s1 == *s2++) {
        if (*s1++ == '\0') {
            return 0;
        }
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)(s2 - 1);
}
