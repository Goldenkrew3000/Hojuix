.global _start
_start:
	# Call C main function
	call main

	# Clear RDI, and move C return code (in RAX) to SysV convention (RDI)
	xor %rdi, %rdi
	mov %rax, %rdi

	# Run SYS_EXIT
	mov $7, %rax
	int $0x80
