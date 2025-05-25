#ifndef _SPINLOCK_H
#define _SPINLOCK_H
#include <stdatomic.h>

#define SPINLOCK_INIT { .lock = ATOMIC_FLAG_INIT }

typedef struct {
    atomic_flag lock;
} spinlock_t;

void spinlock_acquire(atomic_flag* lock);
void spinlock_release(atomic_flag* lock);

#endif
