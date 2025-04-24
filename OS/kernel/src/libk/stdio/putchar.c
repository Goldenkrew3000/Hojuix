#include <stdio.h>
#include <drivers/framebuffer.h>
#include <drivers/rs232.h>

int putchar(int ic) {
	char c = (char) ic;
	framebuffer_putchar(c);
	return ic;
}
