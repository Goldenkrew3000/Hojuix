# Hojuix LibC

# Use the GCC 14.2.0 x86_64-elf cross compiler
export PATH="/home/user/Developer/Hojuix/CrossCompiler/output/bin:$PATH"

CC="x86_64-elf-gcc"
AR="x86_64-elf-ar"
CFLAGS="-m64 -std=gnu11 -g -ffreestanding -Wall -Wextra -fno-stack-protector -fno-stack-check"
INCLUDE="-I common -I include" # Include GCC common headers and Hojuix LibC headers

$CC -MD -c sys/syscalls.S $CFLAGS $INCLUDE -o sys/syscalls.o
$CC -MD -c sys/syscall_interface.c $CFLAGS $INCLUDE -o sys/syscall_interface.o

$CC -MD -c stdio/printf.c $CFLAGS $INCLUDE -o stdio/printf.o
$CC -MD -c stdio/putchar.c $CFLAGS $INCLUDE -o stdio/putchar.o
$CC -MD -c stdio/puts.c $CFLAGS $INCLUDE -o stdio/puts.o

$CC -MD -c stdlib/abs.c $CFLAGS $INCLUDE -o stdlib/abs.o

$AR rcs libc.a \
sys/syscalls.o \
sys/syscall_interface.o \
stdio/putchar.o \
stdio/printf.o \
stdio/puts.o \
stdlib/abs.o \
