#include <stdio.h>
#include <cpuid.h>

int main() {
    unsigned int eax, ebx, ecx, edx;
    char brand[0x40]; // 64 bytes to hold the brand string

    // Get the CPU brand string
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

    // Ensure the string is null-terminated
    brand[48] = '\0';

    // Print the CPU brand string
    printf("CPU Brand: %s\n", brand);

    return 0;
}

