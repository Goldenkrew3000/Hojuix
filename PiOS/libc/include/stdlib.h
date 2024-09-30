#ifndef _STDLIB_H
#define _STDLIB_H 1

#include <sys/cdefs.h>

__attribute__((__noreturn__))
void abort(void);

// musl 1.2.5
int abs (int);

#endif
