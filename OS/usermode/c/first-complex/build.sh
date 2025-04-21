gcc -ffreestanding -nostdlib -O0 -c main.c -o main.o
as --64 -o start.o start.S
ld -T linker.ld -o syscall.elf start.o main.o
