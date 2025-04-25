# Use the GCC 14.2.0 x86_64-elf cross compiler
export PATH="/home/user/Developer/Hojuix/CrossCompiler/output/bin:$PATH"

CC="x86_64-elf-gcc"
AS="x86_64-elf-as"
LD="x86_64-elf-ld"

CFLAGS="-g -nostdinc -ffreestanding -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2"
#LINK="-L ../../libc"
INCL="-I ../../libc/include -I ../../libc/common"
LINKFLAGS="-m elf_x86_64 -nostdlib -lc -static -T linker.ld -L ../../libc" # Can find -lc

$AS --64 -o start.o start.S
$CC $CFLAGS $INCL -c cowsay.c -o main.o
$LD $LINKFLAGS -o hello.efi start.o main.o 

    #../../libc/sys/syscalls.o \
    #../../libc/sys/syscall_interface.o \
      #../../libc/stdio/putchar.o \
      #../../libc/stdio/printf.o \
      #../../libc/stdio/puts.o \
