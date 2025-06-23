#include <kern/kprintf.h>
#include <misc/kpanic.h>

__attribute__((__noreturn__))
void kpanic(const char* s) {
    printf("[KERNEL] Kernel Panic: %s", s);
    asm volatile("cli; hlt;");
}
