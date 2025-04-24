#include <process/syscall.h>
#include <drivers/ps2_keyboard.h>
#include <drivers/acpi.h>
#include <kernel.h>
#include <stdio.h>

// C sysV ABI: RDI, RSI, RDX, RCX, R8, R9, then stack
void syscall_handler(syscall_regs_t* r) {
    //printf("[SYS_WRITE] RAX: %d, RBX: 0x%x, RDI: %d, RSI: 0x%x\n", r->rax, r->rbx, r->rdi, r->rsi);
    if (r->rax == 1) {
        syscall_print((char*)r->rbx);
        goto sysv_convention_clean;
    } else if (r->rax == 3) {
        syscall_exit(r->rdi);
    } else if (r->rax == 200) {
        // SYS_INPUT (Temporary syscall to receive keyboard input)
        char c = receive_keyboard_input();
        r->rax = c;
    } else if (r->rax = 48) {
        // SYS_SHUTDOWN
        acpi_shutdown();
    }
finish_syscall:
    return; // Return to assembly code
sysv_convention_clean:
    // Only runs to here if called, clean the SysV convention argument space
    // Cannot do this in the usermode libc, needs to only happen on some syscalls
    // NOTE: This is in order: Syscall, Arg1-6
    r->rax = 0;
    r->rdi = 0;
    r->rsi = 0;
    r->rdx = 0;
    r->r10 = 0;
    r->r8 = 0;
    r->r9 = 0;
    goto finish_syscall;
}

int syscall_print(char* c) {
    printf("%c", c);
    return 0;
}

// SYS_EXIT - Do NOT return, WILL Page Fault
void syscall_exit(int rc) {
    //printf("EXIT SYSCALL CALLED\n")
    printf("[SYS_EXIT] Return code: %d\n", rc);
    kernel_finished();
}
