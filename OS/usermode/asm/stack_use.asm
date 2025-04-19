section .text
global _start:

_start:
	mov eax, 5
	mov ebx, 8
	imul eax, ebx
	push eax ; OH FUCK YEAHHH USER MAPPED STACK IS WORKING!!!
