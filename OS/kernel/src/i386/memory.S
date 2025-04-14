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
    #movw $(4*8)|3, %ax
    #movw %ax, %ds
    #movw %ax, %es
    #movw %ax, %fs
    #movw %ax, %gs

    #movq %rsp, %rax
    #pushq $(4*8)|3           # SS
    #pushq %rax               # RSP
    #pushfq                   # RFLAGS
    #pushq $(3*8)|3           # CS

    #pushq $user_entry # RIP

    #movabs $0x6000000000, %r15  # Load 64-bit immediate into RAX
    #pushq  %r15                  # Push RAX onto the stack

    #iretq


    /* Set up user data segments */
    #mov $0x28 | 3, %ax    /* 0x28 = GDT entry 5 (data), RPL=3 */
    movw $(4*8)|3, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs          /* SS is handled by IRETQ */

    /* Set up IRETQ stack frame */
    mov $0x700000000000, %rax   /* User stack (RSP) */
    pushq $0x28 | 3             /* SS (user data segment, RPL=3) */
    pushq %rax                  /* RSP (user stack pointer) */
    pushfq                      /* RFLAGS (IF=1, interrupts enabled) */
    orq $0x200, (%rsp)          /* Set IF (interrupt flag) */
    pushq $0x20 | 3             /* CS (user code segment, RPL=3) */

    /* Push 64-bit RIP (takes two steps) */
    movabs $0x6000000000, %rax  /* Load full 64-bit address */
    pushq %rax                  /* Now we can push it */

    /* Jump to user mode */
    iretq
