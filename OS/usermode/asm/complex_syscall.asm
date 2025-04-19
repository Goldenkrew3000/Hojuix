section .data
msg db 'Hello from a system call!', 0x0A
len equ $ - msg

section .text
global _start:

_start:
	mov eax, 1		; syscall_print (FOR TESTING PURPOSES - THIS NAME IS STUPID AF)
	mov ebx, msg		; message pointer
	mov ecx, len		; message length
	int 0x80		; apparently a system call
