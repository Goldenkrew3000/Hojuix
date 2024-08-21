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

    # This seems to be causing a triple fault when booting from grub
    #mov $0x10, %eax
    #mov %eax, %ds
    #mov %eax, %es
    #mov %eax, %fs
    #mov %eax, %gs
    #mov %eax, %ss

    # Disable Interrupts
    cli

    # Enable A20 Gate HERE TODO

    # Load the GDT
    lgdt gdt_desc

    # Set Protection Enable Flag in CR0
    # NOTE: When using multiboot, grub puts the info into EAX and EBX. Therefore I cannot use EAX to set CR0
    # Grub also seems to enable 32 bit mode for us automatically though
    #mov %cr0, %eax
    #or $0x1, %eax
    #mov %eax, %cr0

    # Perform Far Jump to Protected Mode
    jmp $CODE_SEG, $prot_mode

    # Hang if the kernel (kernel_main) unexpectedly returns
    cli
1:  hlt
    jmp 1b
.size _start, . - _start

.code32
prot_mode:
    # Protected Mode??

    push %eax
    push %ebx

    call _init
    call kernel_main

    # NOTE: Probably shouldnt return to 16 bit mode in 32 bit mode lmao
    # Halting instead, if not, it causes Triple Fault, but only because kernel is dumb fuck and while loop fixes it
    hlt

gdt_start:
    # Null Descriptor
    .quad 0x00000000 # dq 0
gdt_code:
    # 32 Bit Code Segment
    .word 0xFFFF # dw 0
    .word 0 # dw 0
    .byte 0 # db 0
    .byte 0x9A # db 10011010b
    .byte 0xCF # db 00001111b
    .byte 0 # db 0

    # Explaination:
    # 0xFFFF --> Limit (Lower 16 Bytes)
    # 0 --> Base (Bits 0-15)
    # 0 --> Base (Bits 16-23)
    # 0x9A --> 10011010 --> Access (Present, Ring 0, Code Segment, Executable, Direction 0, Readable)
    # 0xCF --> Granularity (4K Pages, 32-Bit Protected Mode) + Limit (Bits 16-19)
    # 0 --> Base High

    # 32 Bit Code Segment
    .word 0xFFFF
    .word 0
    .byte 0
    .byte 0x92
    .byte 0xCF
    .byte 0

    # 16 Bit Code Segment
    .word 0xFFFF
    .word 0
    .byte 0
    .byte 0x9A
    .byte 0xCF
    .byte 0

    # 16 Bit Code Segment
    .word 0xFFFF
    .word 0
    .byte 0
    .byte 0x92
    .byte 0xCF
    .byte 0
gdt_end:

gdt_desc:
    .word gdt_end - gdt_start - 1
    .long gdt_start

CODE_SEG = gdt_code - gdt_start
