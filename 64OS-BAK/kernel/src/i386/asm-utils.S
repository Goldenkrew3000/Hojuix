.section .text
.code64

.global krnl_halt
krnl_halt:
    cli
    hlt


.global load_gdt, @function
load_gdt:
    lgdt(%rdi)

    # Not sure why, but if I dont pop and push rdi, it page faults
    popq %rdi
    push $0x08 # Push kernel code segment
    pushq %rdi

    # Perform far jump
    lea farjump(%rip), %rax
    push %rax

    ret

load_gdt_old:
    lgdt (%rdi) # Load GDT from the address in RDI

    # Not sure if it's meant to be eax or ax, but all examples point to ax
    mov $0x10, %ax # Load the Kernel Data Segment
    mov %ax, %ds 
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    # uhhh https://github.com/Tix3Dev/KnutOS/blob/30b71e99912de6c2bbe21a7313fceb4ace55bcb5/src/kernel/gdt/gdt_asm.s#L16
    popq %rdi # Pop return address?
    mov $0x08, %rax
    pushq %rax
    pushq %rdi # Push return address?

    lea farjump(%rip), %rax
    pushq $farjump
    #lretq

    # Perform far jump to switch code segment
    #mov $0x8, %rax
    #pushq %rax

    #pushq $0x08 # Push Kernel Code Segment
    #lea farjump(%rip), %rax # Load address of next function into RAX
    #pushq $farjump # Push the address onto the stack
    #lretq # Long return to switch code segment
    #jmpq $0x08,$farjump






farjump:
    mov $0x10, %eax # Load the Kernel Data Segment
    mov %eax, %ds 
    mov %eax, %es
    mov %eax, %fs
    mov %eax, %gs
    mov %eax, %ss
    ret

# NOTE FOR FUTURE. TSS seems the same
#https://github.com/NSG650/Polaris/blob/69c5f8d58ad3f04e0b581bbfbac751ee92e8df4b/kernel/arch/x86_64-pc/sys/gdt_.asm#L2
