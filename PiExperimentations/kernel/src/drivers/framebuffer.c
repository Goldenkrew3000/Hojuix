#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <drivers/framebuffer.h>

unsigned int width;
unsigned int height;
unsigned int pitch;
unsigned int isrgb;
unsigned char *fb; // Raw framebuffer address

void framebuffer_init() {
}
