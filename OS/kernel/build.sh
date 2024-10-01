CC="gcc"
LD="ld"
OBJCPY="objcopy"

CFLAGS="-g -pipe -Wall -Wextra -std=gnu11 -nostdinc -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC -ffunction-sections -fdata-sections -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -isystem freestanding-headers"
LINK="-L ../libc"
INCL="-I ../libc/include -I ../common -I src/include"

LINKFLAGS="-m elf_x86_64 -nostdlib -lk -static -z max-page-size=0x1000 -gc-sections -T linker-x86_64.ld -L ../libc obj/unifont.o"

mkdir obj
mkdir obj/i386
mkdir obj/kernel
mkdir obj/memory
mkdir obj/drivers

# Btw .S asm is built the same way as .c
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/io.c -o obj/i386/io.o

$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/gdt.c -o obj/i386/gdt.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/idt.c -o obj/i386/idt.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/irq.c -o obj/i386/irq.o

$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/acpi.c -o obj/drivers/acpi.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/pci.c -o obj/drivers/pci.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/ahci.c -o obj/drivers/ahci.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/rtc.c -o obj/drivers/rtc.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/rs232.c -o obj/drivers/rs232.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/framebuffer.c -o obj/drivers/framebuffer.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/ps2_keyboard.c -o obj/drivers/ps2_keyboard.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/pit_timer.c -o obj/drivers/pit_timer.o

$CC $CFLAGS $LINK $INCL -MMD -MP -c src/memory/pmmgr.c -o obj/memory/pmmgr.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/memory/vmmgr.c -o obj/memory/vmmgr.o

$CC $CFLAGS $LINK $INCL -MMD -MP -c src/kernel/cc-runtime.c -o obj/kernel/cc-runtime.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/kernel/kernel.c -o obj/kernel/kernel.o

${OBJCPY} -O elf64-x86-64 -B i386:x86-64 -I binary unifont.sfn obj/unifont.o
readelf -s --wide obj/unifont.o

${LD} \
obj/i386/io.o \
obj/i386/gdt.o \
obj/i386/idt.o \
obj/i386/irq.o \
obj/drivers/acpi.o \
obj/drivers/pci.o \
obj/drivers/ahci.o \
obj/drivers/rtc.o \
obj/drivers/rs232.o \
obj/drivers/framebuffer.o \
obj/drivers/ps2_keyboard.o \
obj/drivers/pit_timer.o \
obj/memory/pmmgr.o \
obj/memory/vmmgr.o \
obj/kernel/cc-runtime.o \
obj/kernel/kernel.o \
$LINKFLAGS -o kernel.macho
