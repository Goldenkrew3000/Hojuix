#include <kernel/process/syscall.h>
#include <kernel.h>
#include <stdio.h>

// C sysV ABI: RDI, RSI, RDX, RCX, R8, R9, then stack
void syscall_handler(syscall_regs_t* r) {
    //printf("SYSCALL: %d, RCX: %llx\n", r->rbx, r->rcx);
    if (r->rbx == 1) {
        syscall_print((char*)r->rcx);
    } else if (r->rbx == 2) {
        syscall_newline();
    } else if (r->rbx == 3) {
        syscall_exit(r->rdi);
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

// SYS_EXIT - Do NOT return, WILL Page Fault
void syscall_exit(int rc) {
    //printf("EXIT SYSCALL CALLED\n")
    printf("[SYS_EXIT] Return code: %d\n", rc);
    kernel_finished();
}
