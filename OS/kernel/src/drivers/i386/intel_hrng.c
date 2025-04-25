#include <stdint.h>
#include <drivers/i386/intel_hrng.h>

// TODO Finish this and add documentation

// 325462-sdm-vol-1-2abcd-3abcd-4.pdf Page 202-203
uint64_t intel_rdseed() {
    uint64_t rand_val = 0;
    asm volatile("rdseed %0" : "=r"(rand_val));
    return rand_val;
}

// 325462-sdm-vol-1-2abcd-3abcd-4.pdf Page 202
uint64_t intel_rdrand() {
    //
}
