#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <kernel/util.h>

size_t kern_strlen(const char* string) {
    size_t length = 0;
    while (string[length]) {
        length++;
    }
    return length;
}
