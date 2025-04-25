#include <sys/syscalls.h>

char syscall_input() {
    char c = libc_syscall(SYS_KBD_INPUT);
    return c;
}
