#include <stdio.h>

#if defined(__is_libk)
#include <drivers/framebuffer.h>
#include <drivers/rs232.h>
#endif

int putchar(int ic) {
#if defined(__is_libk)
	char c = (char) ic;
	framebuffer_putchar(c);
	//rs232_write(0x3F8, c);
#else
	// TODO: Implement stdio and the write system call.
#endif
	return ic;
}
