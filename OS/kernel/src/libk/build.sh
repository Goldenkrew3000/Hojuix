# Hojuix LibK build script

# Use GCC 14.2.0 x86_64-elf cross compiler
export PATH="/home/user/Developer/Hojuix/CrossCompiler/output/bin:$PATH"
CC="x86_64-elf-gcc"
AR="x86_64-elf-ar"
CFLAGS="-m64 -std=gnu11 -g -ffreestanding -Wall -Wextra -fno-stack-protector -fno-stack-check"
INCLUDE="-I ../../../common -I ../include -I include"

# Kernel STDIO
$CC -MD -c stdio/printf.c $CFLAGS $INCLUDE -o stdio/printf.o
$CC -MD -c stdio/putchar.c $CFLAGS $INCLUDE -o stdio/putchar.o
$CC -MD -c stdio/puts.c $CFLAGS $INCLUDE -o stdio/puts.o

# Kernel STDLIB
$CC -MD -c stdlib/abs.c $CFLAGS $INCLUDE -o stdlib/abs.o

# Kernel STRING
$CC -MD -c string/memcmp.c $CFLAGS $INCLUDE -o string/memcmp.o
$CC -MD -c string/memcpy.c $CFLAGS $INCLUDE -o string/memcpy.o
$CC -MD -c string/memmove.c $CFLAGS $INCLUDE -o string/memmove.o
$CC -MD -c string/memset.c $CFLAGS $INCLUDE -o string/memset.o
$CC -MD -c string/strlen.c $CFLAGS $INCLUDE -o string/strlen.o
$CC -MD -c string/strcmp.c $CFLAGS $INCLUDE -o string/strcmp.o
$CC -MD -c string/strncmp.c $CFLAGS $INCLUDE -o string/strncmp.o

$AR rcs libk.a \
stdio/printf.o \
stdio/putchar.o \
stdio/puts.o \
stdlib/abs.o \
string/memcmp.o \
string/memcpy.o \
string/memmove.o \
string/memset.o \
string/strlen.o \
