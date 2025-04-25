/*
// Hojuix LibC
// Syscall Handler
*/

/*
// Calling Convention
// RAX -> Syscall
// RBX -> Argument 1
*/

// SysV Calling Convention (Functions)
// (RAX - syscall number), RDI, RSI, RDX, RCX, R8, R9

.global libc_syscall
libc_syscall:
    # Shift arguments down 1 to fit SysV convention
    mov %rdi, %rax
    mov %rsi, %rdi

    # Perform Syscall
    int $0x80

    ret
