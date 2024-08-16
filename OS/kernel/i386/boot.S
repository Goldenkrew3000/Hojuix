.set ALIGN,     1<<0
.set MEMINFO,   1<<1
.set FLAGS,     ALIGN | MEMINFO
.set MAGIC,     0x1BADB002
.set CHECKSUM,  -(MAGIC + FLAGS)

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM
.space 4 * 4

# Reserve a stack for the initial thread
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16KB
stack_top:

# Kernel entry point
.section .text
.global _start
.type _start, @function
_start:
    # Setup Stack
    mov $stack_top, %esp

    # Disable Interrupts
    cli

    # Push EAX and EBX to the stack (Contains Multiboot details)
    push %eax
    push %ebx

    # Call Kernel
    call _init
    call kernel_main

    # Hang if the kernel (kernel_main) unexpectedly returns
    cli
1:  hlt
    jmp 1b
.size _start, . - _start
