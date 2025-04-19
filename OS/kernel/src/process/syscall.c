#include <kernel/process/syscall.h>
#include <stdio.h>

// C sysV ABI: RDI, RSI, RDX, RCX, R8, R9, then stack
void syscall_handler(syscall_regs_t* r) {
    //printf("SYSCALL: %d, RCX: %llx\n", r->rbx, r->rcx);
    if (r->rbx == 1) {
        syscall_print((char*)r->rcx);
    } else if (r->rbx == 2) {
        syscall_newline();
    }
}

int syscall_print(char* c) {
    printf("%c", c);
    return 0;
}

int syscall_newline() {
    printf("\n");
    return 0;
}
