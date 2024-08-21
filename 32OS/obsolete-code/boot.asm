bits 16

global start
start:
    ; Setup stack
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax
    ;mov sp, 0x7C00

    ; Switch to 32 bit Protected mode
    cli ; Disable interrupts
    call EnableA20 ; Enable A20 Gate
    ;call LoadGDT ; Load GDT

    ; Set Protection Enable flag in CR0
    mov eax, cr0
    or al, 1
    mov cr0, eax

    ; Perform Far Jump into protected mode
    jmp dword 08h:.pmode

.pmode:
    ; Protected Mode!!!
    [bits 32]

    ; Setup segment registers
    mov eax, 0x10
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
    mov ss, eax

    jmp .done

    ; Load Kernel
    ;call kernel_main

    ;mov esi, g_Hello
    ;#mov edi, ScreenBuffer
    ;#cld

;#.loop:
;#    lodsb
;#    or al, al
;#    jz .done
;#
;#    mov [edi], al
;#    inc edi
;#    mov [edi], byte 0x47
;#    inc edi
;#    jmp .loop

.done:
    ; Halt
    jmp $
    

EnableA20:
    [bits 16]

    ; Disable Keyboard
    call A20WaitInput
    mov al, KbdControlDisable
    out KbdControlCMD, al

    ; Read Control Output Port
    call A20WaitInput
    mov al, KbdControlReadCtrlOut
    out KbdControlCMD, al

    call A20WaitInput
    in al, KbdControlDAT
    push eax

    ; Write Control Output Port
    call A20WaitInput
    mov al, KbdControlWriteCtrlOut
    out KbdControlCMD, al

    call A20WaitInput
    pop eax
    or al, 2
    out KbdControlDAT, al

    ; Enable Keyboard
    call A20WaitInput
    mov al, KbdControlEnable
    out KbdControlCMD, al

    call A20WaitInput
    ret

A20WaitInput:
    [bits 16]

    ; Wait until Status bit 2 (Input buffer) is 0
    in al, KbdControlCMD
    test al, 2
    jnz A20WaitInput
    ret

A20WaitOutput:
    [bits 16]

    ; Wait until Status bit 1 (Output buffer) is 1
    in al, KbdControlCMD
    test al, 1
    jz A20WaitOutput
    ret

LoadGDT:
    [bits 16]

    lgdt [g_GDTDesc]
    ret

KbdControlDAT           equ 0x60
KbdControlCMD           equ 0x64
KbdControlDisable       equ 0xAD
KbdControlEnable        equ 0xAE
KbdControlReadCtrlOut   equ 0xD0
KbdControlWriteCtrlOut  equ 0xD1
;;ScreenBuffer            equ 0xB8000

g_GDT:
    ; NULL Descriptor
    dq 0

    ; 32 bit Code segment
    dw 0FFFFh ; Limit (lower 16 bytes) = 0xFFFFFFFF for full 32-bit range
    dw 0      ; Base (bits 0-15) = 0x0
    db 0      ; Base (bits 16-23)
    db 10011010b ; Access (present, ring 0, code segment, executable, direction 0, readable)
    db 11001111b ; Granularity (4k pages, 32-bit pmode) + Limit (bits 16-19)
    db 0 ; Base high

    ; 32 bit Code segment
    dw 0FFFFh
    dw 0
    db 0
    db 10010010b ; Changed executable bit to 0, data segment, writable
    db 11001111b
    db 0

    ; 16 bit Code segment
    dw 0FFFFh
    dw 0
    db 0
    db 10011010b
    db 00001111b ; Granularity is 1b pages, 16-bit pmode
    db 0

    ; 16 bit Code segment
    dw 0FFFFh
    dw 0
    db 0
    db 10010010b
    db 00001111b
    db 0

g_GDTDesc:
    dw g_GDTDesc - g_GDT -1 ; Limit = Size of GDT
    dd g_GDT ; Address of GDT

SECTION .bss
    resb 16384
_sys_stack:

;times 510-($-$$) db 0
;dw 0xAA55
