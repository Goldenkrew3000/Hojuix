#include <stdio.h>
#include <sys/syscalls.h>

int putchar(int ic) {
    // Use the SYS_WRITE syscall
    libc_syscall(SYS_PRINTF, ic);
	return ic;
}
