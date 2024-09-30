#include <string.h>
#include <stdint.h>

#include <stdio.h>

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void kern_memset(uint8_t *array, uint8_t value, size_t size) {
    for (size_t i = 0; i < size; i++) {
        array[i] = value;
    }
}

// This function is the same as [ kern_memset ], but sets uint32_t instead of uint8_t, leading to a 4x speed increase when setting large chunks of memory (Such as 8-32gb on boot)
void kern_memset_word(uint8_t *array, uint32_t value, size_t size) {
    printf("Arr: %p\n", array);
    for (size_t i = 0; i < size / 4; i++) {
        array[i * 4] = value;
    }
}
