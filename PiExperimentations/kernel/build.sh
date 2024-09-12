CFLAGS="-Wall -g -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd -Isrc/include -I../common -I ../libc/include -L ../libc"

mkdir obj
mkdir obj/kernel
mkdir obj/drivers

clang --target=aarch64-elf $CFLAGS -c src/armstub.S -o obj/armstub.o

clang --target=aarch64-elf $CFLAGS -c src/kernel/kernel.c -o obj/kernel/kernel.o
clang --target=aarch64-elf $CFLAGS -c src/drivers/uart.c -o obj/drivers/uart.o

ld.lld -m aarch64elf -nostdlib -lk -static -L../libc \
obj/armstub.o \
obj/kernel/kernel.o \
obj/drivers/uart.o \
-T linker.ld -o kernel8.elf

llvm-objcopy -O binary kernel8.elf kernel8.img
