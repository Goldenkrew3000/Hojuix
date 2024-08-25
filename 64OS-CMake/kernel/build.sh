CC="gcc"
LD="ld.lld"

CFLAGS="-g -pipe -Wall -Wextra -std=gnu11 -nostdinc -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC -ffunction-sections -fdata-sections -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -isystem freestanding-headers"
LINK="-L ../libc"
INCL="-I ../libc/include -I ../common -I src/include"

LINKFLAGS="-m elf_x86_64 -nostdlib -lk -static -z max-page-size=0x1000 -gc-sections -T linker-x86_64.ld -L ../libc unifont.o"

mkdir obj
mkdir obj/i386
mkdir obj/kernel

$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/asm-utils.S -o obj/i386/asm-utils.S.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/dt.c -o obj/i386/dt.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/framebuffer.c -o obj/i386/framebuffer.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/io.c -o obj/i386/io.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/keyboard.c -o obj/i386/keyboard.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/pmm.c -o obj/i386/pmm.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/shell.c -o obj/i386/shell.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/timer.c -o obj/i386/timer.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/utils.c -o obj/i386/utils.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/vmm.c -o obj/i386/vmm.o

$CC $CFLAGS $LINK $INCL -MMD -MP -c src/kernel/cc-runtime.c -o obj/kernel/cc-runtime.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/kernel/kernel.c -o obj/kernel/kernel.o

ld \
obj/i386/asm-utils.S.o \
obj/i386/dt.o \
obj/i386/framebuffer.o \
obj/i386/io.o \
obj/i386/keyboard.o \
obj/i386/pmm.o \
obj/i386/shell.o \
obj/i386/timer.o \
obj/i386/utils.o \
obj/i386/vmm.o \
obj/kernel/cc-runtime.o \
obj/kernel/kernel.o \
$LINKFLAGS -o kernel.macho
