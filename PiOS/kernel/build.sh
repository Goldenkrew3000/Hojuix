CFLAGS="-Wall -g -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd -Isrc/include -I../common -I ../libc/include -L ../libc"

mkdir obj
mkdir obj/kernel
mkdir obj/drivers
mkdir obj/memory

clang --target=aarch64-elf $CFLAGS -c src/armstub.S -o obj/armstub.o

clang --target=aarch64-elf $CFLAGS -c src/kernel/kernel.c -o obj/kernel/kernel.o
clang --target=aarch64-elf $CFLAGS -c src/kernel/cc-runtime.c -o obj/kernel/cc-runtime.o
clang --target=aarch64-elf $CFLAGS -c src/drivers/uart.c -o obj/drivers/uart.o
clang --target=aarch64-elf $CFLAGS -c src/drivers/mbox.c -o obj/drivers/mbox.o
clang --target=aarch64-elf $CFLAGS -c src/drivers/rand.c -o obj/drivers/rand.o
clang --target=aarch64-elf $CFLAGS -c src/drivers/framebuffer.c -o obj/drivers/framebuffer.o
clang --target=aarch64-elf $CFLAGS -c src/drivers/timer.c -o obj/drivers/timer.o

clang --target=aarch64-elf $CFLAGS -c src/memory/mmu.c -o obj/memory/mmu.o

ld.lld -m aarch64elf -nostdlib -lk -static -L../libc \
obj/armstub.o \
obj/kernel/kernel.o \
obj/kernel/cc-runtime.o \
obj/drivers/uart.o \
obj/drivers/mbox.o \
obj/drivers/rand.o \
obj/drivers/framebuffer.o \
obj/drivers/timer.o \
obj/memory/mmu.o \
-T linker.ld -o kernel8.elf

llvm-objcopy -O binary kernel8.elf kernel8.img
