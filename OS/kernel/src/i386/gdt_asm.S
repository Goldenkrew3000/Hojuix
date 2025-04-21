.section .text

# GDT Code (Unused)

.global gdt_apply
gdt_apply:
    lgdt (%rip)                     # Load the new GDT
    push $0x08                      # Push code segment to stack (0x08 is Kernel Mode Code Segment)
    leaq .gdt_farjump(%rip), %rax   # Load address of gdt_farjump into %RAX
    push %rax                       # Push the value onto the stack
    retfq                           # Perform a far return

.gdt_farjump:
    # Reload data segment registers (0x10 is Kernel Mode Data Segment)
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

# IDT/ISR Code

.global isr_stub_0
isr_stub_0:
    push $0  // Dummy error code
    push $0  // Interrupt number
    jmp common_interrupt_handler

.global isr_stub_1
isr_stub_1:
    push $0  // Dummy error code
    push $1  // Interrupt number
    jmp common_interrupt_handler

.global isr_stub_2
isr_stub_2:
    push $0  // Dummy error code
    push $2  // Interrupt number
    jmp common_interrupt_handler

.global isr_stub_3
isr_stub_3:
    push $0  // Dummy error code
    push $3  // Interrupt number
    jmp common_interrupt_handler

.global isr_stub_4
isr_stub_4:
    push $0  // Dummy error code
    push $4  // Interrupt number
    jmp common_interrupt_handler

.global isr_stub_5
isr_stub_5:
    push $0  // Dummy error code
    push $5  // Interrupt number
    jmp common_interrupt_handler

.global isr_stub_6
isr_stub_6:
    push $0  // Dummy error code
    push $6  // Interrupt number
    jmp common_interrupt_handler

.global isr_stub_7
isr_stub_7:
    push $0  // Dummy error code
    push $7  // Interrupt number
    jmp common_interrupt_handler

.global isr_stub_8
isr_stub_8:
    push $0  // Dummy error code
    push $8  // Interrupt number
    jmp common_interrupt_handler

.global isr_stub_9
isr_stub_9:
    push $0  // Dummy error code
    push $9  // Interrupt number
    jmp common_interrupt_handler

.global isr_stub_10
isr_stub_10:
    push $0  // Dummy error code
    push $10  // Interrupt number
    jmp common_interrupt_handler

.global isr_stub_11
isr_stub_11:
    push $0  // Dummy error code
    push $11  // Interrupt number
    jmp common_interrupt_handler

.global isr_stub_12
isr_stub_12:
    push $0  // Dummy error code
    push $12  // Interrupt number
    jmp common_interrupt_handler

.global isr_stub_13
isr_stub_13:
    //push $0  // Dummy error code
    push $13  // Interrupt number
    jmp common_interrupt_handler

.global isr_stub_14
isr_stub_14:
    push $0  // Dummy error code
    push $14  // Interrupt number
    jmp common_interrupt_handler

.global isr_stub_other
isr_stub_other:
    push $0  // Dummy error code
    push $15  // Interrupt number
    jmp common_interrupt_handler

.global isr_stub_syscall
isr_stub_syscall:
    jmp syscall_handler_asm

.global isr_stub_table
isr_stub_table:
    .quad isr_stub_0
    .quad isr_stub_1
    .quad isr_stub_2
    .quad isr_stub_3
    .quad isr_stub_4
    .quad isr_stub_5
    .quad isr_stub_6
    .quad isr_stub_7
    .quad isr_stub_8
    .quad isr_stub_9
    .quad isr_stub_10
    .quad isr_stub_11
    .quad isr_stub_12
    .quad isr_stub_13
    .quad isr_stub_14
    .rept 17
    .quad isr_stub_other # Rest of the reserved ones
    .endr
    .rept 96
    .quad isr_stub_other
    .endr
    .quad isr_stub_syscall # 0x80 SYSCALL
    .rept 127
    .quad isr_stub_other
    .endr


.global common_interrupt_handler
common_interrupt_handler:
    # Push everything to the stack
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rbp
    push %rdi
    push %rsi
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15

	cld # Clears the 'direction' flag
	mov %rsp, %rdi
	xor %rbp, %rbp
	call isr_handle

    # Pop everything from the stack
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rsi
    pop %rdi
    pop %rbp
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax

    add $24, %rsp
    #add $16, %rsp
	iretq

.global syscall_handler_asm
syscall_handler_asm:
    # Push everything to the stack
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rbp
    push %rdi
    push %rsi
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15

	cld
	mov %rsp, %rdi
	xor %rbp, %rbp
	call syscall_handler

    # Pop everything from the stack
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rsi
    pop %rdi
    pop %rbp
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax

    #add $24, %rsp
	iretq
