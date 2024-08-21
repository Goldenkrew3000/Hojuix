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
.global tss_entry
tss_entry:
.skip 4
tss_esp0:
.skip 8
tss_esp1:
.skip 104
stack_bottom:
.skip 16384 # 16KB
.global stack_top
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

.global load_gdt
load_gdt:
    # Function to load the new GDT
    movl 4(%esp), %eax
    lgdt (%eax)
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ljmp $0x08,$farjump
farjump:
    ret

.global load_idt
load_idt:
    # Function to load the IDT
    movl 4(%esp), %eax
    lidt (%eax)
    ret

.global load_tss
load_tss:
    # Function to load the TSS
    movw $0x2B, %ax
    ltr %ax
    ret

.global load_page_directory
load_page_directory:
    push %ebp
    mov %esp, %ebp
    mov 8(%esp), %eax
    mov %eax, %cr3
    mov %ebp, %esp
    pop %ebp
    ret

.global isr0
isr0:
    cli
    push $0
    push $0
    jmp isr_common_stub

.global isr1
isr1:
    cli
    push $0
    push $1
    jmp isr_common_stub

.global isr2
isr2:
    cli
    push $0
    push $2
    jmp isr_common_stub

.global isr3
isr3:
    cli
    push $0
    push $3
    jmp isr_common_stub

.global isr4
isr4:
    cli
    push $0
    push $4
    jmp isr_common_stub

.global isr5
isr5:
    cli
    push $0
    push $5
    jmp isr_common_stub

.global isr6
isr6:
    cli
    push $0
    push $6
    jmp isr_common_stub

.global isr7
isr7:
    cli
    push $0
    push $7
    jmp isr_common_stub

.global isr8
isr8:
    cli
    push $8
    jmp isr_common_stub

.global isr9
isr9:
    cli
    push $0
    push $9
    jmp isr_common_stub

.global isr10
isr10:
    cli
    push $10
    jmp isr_common_stub

.global isr11
isr11:
    cli
    push $11
    jmp isr_common_stub

.global isr12
isr12:
    cli
    push $12
    jmp isr_common_stub

.global isr13
isr13:
    cli
    push $13
    jmp isr_common_stub

.global isr14
isr14:
    cli
    push $14
    jmp isr_common_stub

.global isr15
isr15:
    cli
    push $0
    push $15
    jmp isr_common_stub

.global isr16
isr16:
    cli
    push $0
    push $16
    jmp isr_common_stub

.global isr17
isr17:
    cli
    push $0
    push $17
    jmp isr_common_stub

.global isr18
isr18:
    cli
    push $0
    push $18
    jmp isr_common_stub

.global isr19
isr19:
    cli
    push $0
    push $19
    jmp isr_common_stub

.global isr20
isr20:
    cli
    push $0
    push $20
    jmp isr_common_stub

.global isr21
isr21:
    cli
    push $0
    push $21
    jmp isr_common_stub

.global isr22
isr22:
    cli
    push $0
    push $22
    jmp isr_common_stub

.global isr23
isr23:
    cli
    push $0
    push $23
    jmp isr_common_stub

.global isr24
isr24:
    cli
    push $0
    push $24
    jmp isr_common_stub

.global isr25
isr25:
    cli
    push $0
    push $25
    jmp isr_common_stub

.global isr26
isr26:
    cli
    push $0
    push $26
    jmp isr_common_stub

.global isr27
isr27:
    cli
    push $0
    push $27
    jmp isr_common_stub

.global isr28
isr28:
    cli
    push $0
    push $28
    jmp isr_common_stub

.global isr29
isr29:
    cli
    push $0
    push $29
    jmp isr_common_stub

.global isr30
isr30:
    cli
    push $0
    push $30
    jmp isr_common_stub

.global isr31
isr31:
    cli
    push $0
    push $31
    jmp isr_common_stub

# Software Interrupts
.global isr48
isr48:
    cli
    push $0
    push $48
    jmp isr_common_stub

isr_common_stub: # Could REALLY do with fixing TODO but it 'works'
    pusha
    movw %ds,%ax
    push %eax
    push %esp
    movw $0x10,%ax
    movw %ax,%ds
    movw %ax,%es
    movw %ax,%fs
    movw %ax,%gs
    cld
    call isr_fault_handler
    pop %esp
    pop %ebx
    movw %bx,%ds
    movw %bx,%es
    movw %bx,%fs
    movw %bx,%gs
    popa
    add $8, %esp
    iret

.global irq0
irq0:
    cli
    push $0
    push $32
    jmp irq_common_stub

.global irq1
irq1:
    cli
    push $0
    push $33
    jmp irq_common_stub

.global irq2
irq2:
    cli
    push $0
    push $34
    jmp irq_common_stub

.global irq3
irq3:
    cli
    push $0
    push $35
    jmp irq_common_stub

.global irq4
irq4:
    cli
    push $0
    push $36
    jmp irq_common_stub

.global irq5
irq5:
    cli
    push $0
    push $37
    jmp irq_common_stub

.global irq6
irq6:
    cli
    push $0
    push $38
    jmp irq_common_stub

.global irq7
irq7:
    cli
    push $0
    push $39
    jmp irq_common_stub

.global irq8
irq8:
    cli
    push $0
    push $40
    jmp irq_common_stub

.global irq9
irq9:
    cli
    push $0
    push $41
    jmp irq_common_stub

.global irq10
irq10:
    cli
    push $0
    push $42
    jmp irq_common_stub

.global irq11
irq11:
    cli
    push $0
    push $43
    jmp irq_common_stub

.global irq12
irq12:
    cli
    push $0
    push $44
    jmp irq_common_stub

.global irq13
irq13:
    cli
    push $0
    push $45
    jmp irq_common_stub

.global irq14
irq14:
    cli
    push $0
    push $46
    jmp irq_common_stub

.global irq15
irq15:
    cli
    push $0
    push $47
    jmp irq_common_stub

irq_common_stub:
    # I am not sure how broken this code is, but it seems to be working (Tested keyboard input)
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %esp, %eax
    push %eax
    mov $irq_handler, %eax
    call %eax
    pop %eax
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    add $8, %esp
    iret