.section .text

.global i386_memset
i386_memset:
    push %rdi
    mov %rsi, %rax
    mov %rdx, %rcx
    rep stosb
    pop %rax
    ret

.global i386_memcpy
i386_memcpy:
    mov %rdx, %rcx
    mov %rdi, %rax
    rep movsb
    ret

# TODO memmove and memcmp are not implemented here yet
