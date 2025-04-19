int main(int argc, char** argv) {
	asm volatile("mov $1, %ebx;\nmov $0x48, %ecx;\nint $0x80;\n");
	asm volatile("mov $1, %ebx;\nmov $0x64, %ecx;\nint $0x80;\n");
	asm volatile("mov $2, %ebx;\nint $0x80;\n");
	return 10;
}
