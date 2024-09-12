#include <stdio.h>

#if defined(__is_libk)
#include <drivers/uart.h>
#endif

int putchar(int ic) {
#if defined(__is_libk)
	char c = (char) ic;
    uart1_send(c);
	//framebuffer_putchar(c);
#else
	// TODO: Implement stdio and the write system call.
#endif
	return ic;
}
