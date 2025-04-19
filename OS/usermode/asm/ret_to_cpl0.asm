section .text
global _start:

_start:
	push eax
	push ebx
	mov eax, 5
	mov ebx, 8
	imul eax, ebx
	pop ebx
	pop eax
	ret
