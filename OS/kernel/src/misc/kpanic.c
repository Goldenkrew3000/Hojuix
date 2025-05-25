#include <kern/kprintf.h>
#include <misc/kpanic.h>

__attribute__((__noreturn__))
void kpanic(void) {
    printf("[KERNEL] Kernel Panic (abort)!\n");
    asm volatile("cli; hlt;");
}
