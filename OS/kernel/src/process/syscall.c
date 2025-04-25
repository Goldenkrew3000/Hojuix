#include <process/syscall.h>
#include <drivers/ps2_keyboard.h>
#include <drivers/acpi.h>
#include <kernel.h>
#include <stdio.h>

#include <memory/vmmgr.h>
#include <i386/asm_functions.h>
#include <fs/fat16.h>
#include <string.h>

// C sysV ABI: RDI, RSI, RDX, RCX, R8, R9, then stack
void syscall_handler(syscall_regs_t* r) {
    switch (r->rax) {
        case SYS_WRITE:
            // SYS_WRITE
            break;
        case SYS_READ:
            // SYS_READ
            break;
        case SYS_OPEN:
            // SYS_OPEN
            syscall_open();
            break;
        case SYS_CLOSE:
            // SYS_CLOSE
            break;
        case SYS_FORK:
            // SYS_FORK
            break;
        case SYS_EXEC:
            // SYS_EXEC
            syscall_exec((char*)r->rdi); // RDI: Pointer to path
            break;
        case SYS_EXIT:
            // SYS_EXIT
            //syscall_exit(r->rdi); // RDI: Return code
            printf("\n");
            syscall_exec("SHELL.ELF");
            break;
        case SYS_KILL:
            // SYS_KILL
            break;
        case SYS_GETPID:
            // SYS_GETPID
            break;
        case SYS_IOCTL:
            // SYS_IOCTL
            break;
        case SYS_MMAP:
            // SYS_MMAP
            break;
        case SYS_MUNMAP:
            // SYS_MUNMAP
            break;
        case SYS_REBOOT:
            // SYS_REBOOT
            break;
        case SYS_SHUTDOWN:
            // SYS_SHUTDOWN
            acpi_shutdown(); // TODO this is actually a reboot, and to add insult, it doesnt even work lmfao
            break;
        case SYS_PRINTF:
            // Temporary printf solution
            syscall_print((char*)r->rdi);
            goto sysv_convention_clean;
            break;
        case SYS_KBD_INPUT:
            // Temporary keyboard solution
            char c = receive_keyboard_input();
            r->rax = c; // Return char through RAX
            break;
        default:
            // Unknown Syscall
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

// SYS_OPEN
void syscall_open() {
    //
}

// SYS_EXEC - Does NOT return, new machine code in the same place, would fault in some way
void syscall_exec(char* rdi) {
    printf("PATH: %s\n", rdi);
    char path[64] = {0x00};
    i386_memcpy((uint8_t*)path, rdi, strlen(rdi));
    i386_memset((uint8_t*)USER_STACK_ADDR, 0x00, 65536);
    i386_memset(0x800000, 0x00, 32768);
    i386_memset(0x400000, 0x00, 32768);
    printf("PASSING %s\n", path);
    fat16_read_in_exec_file(path);
    run_usermode();
}
