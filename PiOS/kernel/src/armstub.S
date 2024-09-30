.section ".text.boot"

.global _start

_start:
    // Stop all cores except for core 1
    mrs x1, mpidr_el1
    and x1, x1, #3
    cbz x1, 2f
    // CPU ID > 0, halt
1:  wfe
    b 1b
    // CPU ID == 0
    // Set top of stack
2:  ldr x1, =_start
    
    // Setup EL1
    mrs x0, CurrentEL
    and x0, x0, #12 // Clear Reserved Bits

    // Running in EL3?
    cmp x0, #12
    bne 5f

    // Should never be executed, just for completeness
    mov x2, #0x5b1
    msr scr_el3, x2
    mov x2, #0x3c9
    msr spsr_el3, x2
    adr x2, 5f
    msr elr_el3, x2
    eret

    // Running in EL2?
5:  cmp x0, #4
    beq 5f
    msr sp_el1, x1

    // Enable CNTP for EL1
    mrs x0, cnthctl_el2
    orr x0, x0, #3
    msr cnthctl_el2, x0
    msr cntvoff_el2, xzr

    // Enable AArch64 in EL1
    mov x0, #(1 << 31) // AArch64
    orr x0, x0, #(1 << 1) // SWIO Hardwired on Pi3
    msr hcr_el2, x0
    mrs x0, hcr_el2

    // Setup SCTLR Access
    mov x2, #0x0800
    movk x2, #0x30d0, lsl #16
    msr sctlr_el1, x2

    // Change execution level to EL1
    mov x2, #0x3c4
    msr spsr_el2, x2
    adr x2, 5f
    msr elr_el2, x2
    eret

5:  mov sp, x1
    // Clear BSS
    ldr x1, =__bss_start
    ldr w2, =__bss_size
3:  cbz w2, 4f
    str xzr, [x1], #8
    sub w2, w2, #1
    cbnz w2, 3b
4:  // Jump to C Code
    bl main
    // For failsafe, halt this core too
    b 1b
