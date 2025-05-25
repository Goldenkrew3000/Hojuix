#include <stdint.h>
#include <arch/amd64/cpuinfo.h>
#include <kern/kprintf.h>
#include <kernel_ext/cpuid.h>

void print_cpuid() {
    uint32_t eax, ebx, ecx, edx;
    char vendor[13] = {0};
    char brand[49] = {0};

    // Get Vendor ID
    if (__get_cpuid(0, &eax, &ebx, &ecx, &edx)) {
        *((uint32_t*)&vendor[0]) = ebx;
        *((uint32_t*)&vendor[4]) = edx;
        *((uint32_t*)&vendor[8]) = ecx;
    }

    // Get CPU Brand String
    if (__get_cpuid(0x80000002, &eax, &ebx, &ecx, &edx)) {
        *((uint32_t*)&brand[0]) = eax;
        *((uint32_t*)&brand[4]) = ebx;
        *((uint32_t*)&brand[8]) = ecx;
        *((uint32_t*)&brand[12]) = edx;
    }
    if (__get_cpuid(0x80000003, &eax, &ebx, &ecx, &edx)) {
        *((uint32_t*)&brand[16]) = eax;
        *((uint32_t*)&brand[20]) = ebx;
        *((uint32_t*)&brand[24]) = ecx;
        *((uint32_t*)&brand[28]) = edx;
    }
    if (__get_cpuid(0x80000004, &eax, &ebx, &ecx, &edx)) {
        *((uint32_t*)&brand[32]) = eax;
        *((uint32_t*)&brand[36]) = ebx;
        *((uint32_t*)&brand[40]) = ecx;
        *((uint32_t*)&brand[44]) = edx;
    }
    printf("CPU/SoC: (%s) %s\n", vendor, brand);
}
