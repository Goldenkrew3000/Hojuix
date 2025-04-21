.global _start
_start:
	# Call C main function
	call main

	# Clear RDI, and move C return code (in RAX) to SysV convention (RDI)
	xor %rdi, %rdi
	mov %rax, %rdi

	# Run SYS_EXIT
	mov $3, %rbx
	int $0x80
