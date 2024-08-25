// musl 1.2.5 - src/stdlib/abs.c

#include <stdlib.h>

int abs(int a)
{
        return a>0 ? a : -a;
}
