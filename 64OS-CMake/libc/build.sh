# Libk/c builder ported from bare bones 32bit

CC="gcc"
AR="ar"
CFLAGS="-m64 -std=gnu11 -g -ffreestanding -Wall -Wextra"
INCLUDE="-I ../common -I ../kernel/src/include -I include"

$CC -MD -c stdio/printf.c $CFLAGS -D__is_libk $INCLUDE -o stdio/printf.libk.o
$CC -MD -c stdio/putchar.c $CFLAGS -D__is_libk $INCLUDE -o stdio/putchar.libk.o
$CC -MD -c stdio/puts.c $CFLAGS -D__is_libk $INCLUDE -o stdio/puts.libk.o

$CC -MD -c stdlib/abort.c $CFLAGS -D__is_libk $INCLUDE -o stdlib/abort.libk.o
$CC -MD -c stdlib/abs.c $CFLAGS -D__is_libk $INCLUDE -o stdlib/abs.libk.o

$CC -MD -c string/memcmp.c $CFLAGS -D__is_libk $INCLUDE -o string/memcmp.libk.o
$CC -MD -c string/memcpy.c $CFLAGS -D__is_libk $INCLUDE -o string/memcpy.libk.o
$CC -MD -c string/memmove.c $CFLAGS -D__is_libk $INCLUDE -o string/memmove.libk.o
$CC -MD -c string/memset.c $CFLAGS -D__is_libk $INCLUDE -o string/memset.libk.o
$CC -MD -c string/strlen.c $CFLAGS -D__is_libk $INCLUDE -o string/strlen.libk.o

$AR rcs libk.a \
stdio/printf.libk.o \
stdio/putchar.libk.o \
stdio/puts.libk.o \
stdlib/abort.libk.o \
stdlib/abs.libk.o \
string/memcmp.libk.o \
string/memcpy.libk.o \
string/memmove.libk.o \
string/memset.libk.o \
string/strlen.libk.o \
