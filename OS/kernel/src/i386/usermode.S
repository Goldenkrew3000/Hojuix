.section .text

.global launch_usermode
launch_usermode:
    /* 1. Set up user data segments (DPL=3) */
    mov $0x23, %ax    /* User data segment (GDT entry 0x20 | 0x3) */
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs      /* SS will be handled by IRET */

    /* 2. Create IRET stack frame */
    mov %rsp, %rdi    /* Save current stack pointer */

    /* Push SS:RSP:RFLAGS:CS:RIP for IRETQ */
    pushq $0x23       /* SS (user data segment) */
    pushq %r15        /* User stack pointer (passed in R15) */
    pushfq            /* RFLAGS */
    orq $0x200, (%rsp)/* Enable interrupts (IF flag) */
    pushq $0x1b       /* CS (user code segment - GDT entry 0x18 | 0x3) */
    pushq %r14        /* Instruction pointer (passed in R14) */

    /* 3. Perform the mode switch */
    iretq
