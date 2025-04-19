section .text
global _start:

_start:
	mov eax, 5
	mov ebx, 8
	imul eax, ebx
	invlpg [0] ; YESS ITS THROWING 0X13 #GP HELL YEAH!!!
