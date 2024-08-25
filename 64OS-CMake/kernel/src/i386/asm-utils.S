.section .text
.code64

.global krnl_halt
krnl_halt:
    cli
    hlt

# NOTE FOR FUTURE. TSS seems the same
#https://github.com/NSG650/Polaris/blob/69c5f8d58ad3f04e0b581bbfbac751ee92e8df4b/kernel/arch/x86_64-pc/sys/gdt_.asm#L2
