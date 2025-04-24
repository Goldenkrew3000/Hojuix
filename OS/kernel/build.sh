# Hojuix build script

# Use GCC 14.2.0 x86_64-elf cross compiler
export PATH="/home/user/Developer/Hojuix/CrossCompiler/output/bin:$PATH"
CC="x86_64-elf-gcc"
LD="x86_64-elf-ld"
OBJCPY="x86_64-elf-objcopy"

CFLAGS="-g -pipe -Wall -Wextra -std=gnu11 -nostdinc -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC -ffunction-sections -fdata-sections -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -isystem freestanding-headers"
LINK="-L src/libk"
INCL="-I src/libk/include -I ../common -I src/include"

LINKFLAGS="-m elf_x86_64 -nostdlib -lk -static -z max-page-size=0x1000 -gc-sections -T linker-x86_64.ld -L ../libc obj/unifont.o"

# Make needed directories
mkdir obj
mkdir obj/i386
mkdir obj/kernel
mkdir obj/memory
mkdir obj/drivers
mkdir obj/fs
mkdir obj/process
mkdir obj/misc

# i386 / x86_64 specific files
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/gdt.c -o obj/i386/gdt.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/idt.c -o obj/i386/idt.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/irq.c -o obj/i386/irq.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/idt_handler.S -o obj/i386/idt_handler.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/memory_functions.S -o obj/i386/memory_functions.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/usermode.S -o obj/i386/usermode.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/io.c -o obj/i386/io.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/spinlock.c -o obj/i386/spinlock.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/i386/cpuinfo.c -o obj/i386/cpuinfo.o

# Driver files
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/acpi.c -o obj/drivers/acpi.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/pci.c -o obj/drivers/pci.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/ahci.c -o obj/drivers/ahci.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/rtc.c -o obj/drivers/rtc.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/rs232.c -o obj/drivers/rs232.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/framebuffer.c -o obj/drivers/framebuffer.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/ps2_keyboard.c -o obj/drivers/ps2_keyboard.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/pit_timer.c -o obj/drivers/pit_timer.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/ata_pio.c -o obj/drivers/ata_pio.o

# Memory management related files
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/memory/pmmgr.c -o obj/memory/pmmgr.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/memory/vmmgr.c -o obj/memory/vmmgr.o

# Filesystem files
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/fs/fat16.c -o obj/fs/fat16.o

# Usermode related files
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/process/syscall.c -o obj/process/syscall.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/process/elf.c -o obj/process/elf.o

# Misc files
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/misc/kpanic.c -o obj/misc/kpanic.o

# Kernel entry point
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/kernel/cc-runtime.c -o obj/kernel/cc-runtime.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/kernel/kernel.c -o obj/kernel/kernel.o

# SSFN Font
${OBJCPY} -O elf64-x86-64 -B i386:x86-64 -I binary unifont.sfn obj/unifont.o
readelf -s --wide obj/unifont.o

${LD} \
obj/i386/gdt.o \
obj/i386/idt.o \
obj/i386/irq.o \
obj/i386/idt_handler.o \
obj/i386/memory_functions.o \
obj/i386/usermode.o \
obj/i386/io.o \
obj/i386/spinlock.o \
obj/i386/cpuinfo.o \
obj/drivers/acpi.o \
obj/drivers/pci.o \
obj/drivers/ahci.o \
obj/drivers/rtc.o \
obj/drivers/rs232.o \
obj/drivers/framebuffer.o \
obj/drivers/ps2_keyboard.o \
obj/drivers/pit_timer.o \
obj/drivers/ata_pio.o \
obj/memory/pmmgr.o \
obj/memory/vmmgr.o \
obj/fs/fat16.o \
obj/process/syscall.o \
obj/process/elf.o \
obj/misc/kpanic.o \
obj/kernel/cc-runtime.o \
obj/kernel/kernel.o \
$LINKFLAGS -o kernel.macho
