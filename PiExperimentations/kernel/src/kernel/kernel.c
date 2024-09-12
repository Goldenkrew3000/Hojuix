#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <drivers/uart.h>

#include <stdio.h>

void main() {
    uart1_init();


    printf("Hello world from printf in libk!\n");
}
