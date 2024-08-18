#include <stdio.h>
#include <cpuid.h>

int main() {
    unsigned int eax, ebx, ecx, edx;

    // Get information about the CPU using the __cpuid intrinsic
    __cpuid(1, eax, ebx, ecx, edx);

    // Print the CPU information
    printf("CPU Info:\n");
    printf("EAX: %08x\n", eax);
    printf("EBX: %08x\n", ebx);
    printf("ECX: %08x\n", ecx);
    printf("EDX: %08x\n", edx);

    return 0;
}

