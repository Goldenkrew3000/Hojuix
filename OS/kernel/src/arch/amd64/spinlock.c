/*
// Very basic Spinlock
// Based off of: https://wiki.osdev.org/Spinlock
*/

#include <stdatomic.h>
#include <arch/amd64/spinlock.h>

void spinlock_acquire(atomic_flag* lock) {
    while (atomic_flag_test_and_set_explicit(lock, memory_order_acquire)) {
        asm volatile("pause");
    }
}

void spinlock_release(atomic_flag* lock) {
    atomic_flag_clear_explicit(lock, memory_order_release);
}
