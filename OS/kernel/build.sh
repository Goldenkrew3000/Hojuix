# BSD/Hojuix build script

# Use GCC 15.1.0 x86_64-elf cross compiler
export PATH="/home/user/Developer/Hojuix/CrossCompiler/x86_64-output/bin:$PATH"
CC="x86_64-elf-gcc"
LD="x86_64-elf-ld"
OBJCPY="x86_64-elf-objcopy"

# Kernel Object
KERNOBJ="vmbsd"

CFLAGS="-g -pipe -Wall -Wextra -std=gnu11 -nostdinc -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC -ffunction-sections -fdata-sections -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -isystem freestanding-headers"
INCL="-I ../common -I src/include"

LINKFLAGS="-m elf_x86_64 -nostdlib -static -z max-page-size=0x1000 -gc-sections -T linker-x86_64.ld obj/unifont.o"

# Make needed directories
mkdir obj
mkdir obj/arch
mkdir obj/arch/amd64
mkdir obj/kern
mkdir obj/memory
mkdir obj/drivers
mkdir obj/drivers/sound
mkdir obj/drivers/i386
mkdir obj/fs
mkdir obj/process
mkdir obj/misc

# /kern
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/kern/cc-runtime.c -o obj/kern/cc-runtime.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/kern/kern_entry.c -o obj/kern/kern_entry.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/kern/kern_kprintf.c -o obj/kern/kern_kprintf.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/kern/kern_cdefs.c -o obj/kern/kern_cdefs.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/kern/extern_printf.c -o obj/kern/extern_printf.o

# /arch/amd64
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/arch/amd64/gdt.c -o obj/arch/amd64/gdt.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/arch/amd64/idt.c -o obj/arch/amd64/idt.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/arch/amd64/irq.c -o obj/arch/amd64/irq.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/arch/amd64/idt_handler.S -o obj/arch/amd64/idt_handler.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/arch/amd64/memory_functions.S -o obj/arch/amd64/memory_functions.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/arch/amd64/usermode.S -o obj/arch/amd64/usermode.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/arch/amd64/io.c -o obj/arch/amd64/io.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/arch/amd64/spinlock.c -o obj/arch/amd64/spinlock.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/arch/amd64/cpuinfo.c -o obj/arch/amd64/cpuinfo.o

# Driver files
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/acpi.c -o obj/drivers/acpi.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/pci.c -o obj/drivers/pci.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/ahci.c -o obj/drivers/ahci.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/rs232.c -o obj/drivers/rs232.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/framebuffer.c -o obj/drivers/framebuffer.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/ps2_keyboard.c -o obj/drivers/ps2_keyboard.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/nvme.c -o obj/drivers/nvme.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/xhci.c -o obj/drivers/xhci.o

# Sound Driver Files
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/sound/hojuix_hda.c -o obj/drivers/sound/hojuix_hda.o

# i386 Driver files
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/i386/rtc.c -o obj/drivers/i386/rtc.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/i386/pit_timer.c -o obj/drivers/i386/pit_timer.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/drivers/i386/intel_hrng.c -o obj/drivers/i386/intel_hrng.o

# Memory management related files
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/memory/pmmgr.c -o obj/memory/pmmgr.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/memory/vmmgr.c -o obj/memory/vmmgr.o

# Filesystem files
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/fs/ext2.c -o obj/fs/ext2.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/fs/fat16.c -o obj/fs/fat16.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/fs/guid_pt.c -o obj/fs/guid_pt.o

# Usermode related files
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/process/syscall.c -o obj/process/syscall.o
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/process/elf.c -o obj/process/elf.o

# Misc files
$CC $CFLAGS $LINK $INCL -MMD -MP -c src/misc/kpanic.c -o obj/misc/kpanic.o

# SSFN Font
${OBJCPY} -O elf64-x86-64 -B i386:x86-64 -I binary unifont.sfn obj/unifont.o
readelf -s --wide obj/unifont.o

${LD} \
obj/arch/amd64/gdt.o \
obj/arch/amd64/idt.o \
obj/arch/amd64/irq.o \
obj/arch/amd64/idt_handler.o \
obj/arch/amd64/memory_functions.o \
obj/arch/amd64/usermode.o \
obj/arch/amd64/io.o \
obj/arch/amd64/spinlock.o \
obj/arch/amd64/cpuinfo.o \
obj/drivers/acpi.o \
obj/drivers/pci.o \
obj/drivers/ahci.o \
obj/drivers/rs232.o \
obj/drivers/framebuffer.o \
obj/drivers/ps2_keyboard.o \
obj/drivers/nvme.o \
obj/drivers/xhci.o \
obj/drivers/sound/hojuix_hda.o \
obj/drivers/i386/rtc.o \
obj/drivers/i386/pit_timer.o \
obj/drivers/i386/intel_hrng.o \
obj/memory/pmmgr.o \
obj/memory/vmmgr.o \
obj/fs/fat16.o \
obj/fs/ext2.o \
obj/fs/guid_pt.o \
obj/process/syscall.o \
obj/process/elf.o \
obj/misc/kpanic.o \
obj/kern/cc-runtime.o \
obj/kern/kern_cdefs.o \
obj/kern/kern_entry.o \
obj/kern/kern_kprintf.o \
obj/kern/extern_printf.o \
$LINKFLAGS -o $KERNOBJ
