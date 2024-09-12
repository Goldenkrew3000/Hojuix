#include <stdio.h>
#include <string.h>
#include <kernel_ext/cpuid.h>

void print_cpuid() {
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
    char brand[64];

    __cpuid(0x80000002, eax, ebx, ecx, edx);
    memcpy(brand, &eax, 4);
    memcpy(brand + 4, &ebx, 4);
    memcpy(brand + 8, &ecx, 4);
    memcpy(brand + 12, &edx, 4);

    __cpuid(0x80000003, eax, ebx, ecx, edx);
    memcpy(brand + 16, &eax, 4);
    memcpy(brand + 20, &ebx, 4);
    memcpy(brand + 24, &ecx, 4);
    memcpy(brand + 28, &edx, 4);

    __cpuid(0x80000004, eax, ebx, ecx, edx);
    memcpy(brand + 32, &eax, 4);
    memcpy(brand + 36, &ebx, 4);
    memcpy(brand + 40, &ecx, 4);
    memcpy(brand + 44, &edx, 4);

    printf("CPU: %s\n", brand);
}
