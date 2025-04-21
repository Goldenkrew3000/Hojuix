.section .text
.extern user_entry

.global i386_kern_memset
i386_kern_memset:
    push %rdi
    mov %rsi, %rax
    mov %rdx, %rcx
    rep stosb
    pop %rax
    ret

.global i386_kern_memcpy
i386_kern_memcpy:
    mov %rdx, %rcx
    mov %rdi, %rax
    rep movsb
    ret

.global kernel_test
kernel_test:
    call *%rdi
    ret

.global usermode
usermode:
    /* Set up user data segments */
    mov $0x23, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs          /* SS is handled by IRETQ */

    mov %rsp, %rdi

    push $0x23                      # User data segment
    mov $0x700000000000, %r15       # Move user stack pointer address into r15
    push %r15                       # Push user stack pointer
    pushfq
    orq $0x200, (%rsp)
    push $0x1b                      # User code segment

    mov $0x6000000000, %rax         # User mapped code is stored here
    push %rax

    iretq





    /* Set up IRETQ stack frame - HERE FOR REFERENCE */
    #mov $0x700000000000, %rax   /* User stack (RSP) */
    #pushq $0x28 | 3             /* SS (user data segment, RPL=3) */
    #pushq %rax                  /* RSP (user stack pointer) */
    #pushfq                      /* RFLAGS (IF=1, interrupts enabled) */
    #orq $0x200, (%rsp)          /* Set IF (interrupt flag) */
    #pushq $0x20 | 3             /* CS (user code segment, RPL=3) */

    #leaq .user_start, %rax   # Works, but not necessary here

.global usermode2
usermode2:
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
