#include <stdio.h>
#include <stdint.h>

#define ALIGNMENT 4096

// Function to get the next number divisible by 4096
uint32_t next_divisible_by_4096(uint32_t num) {
    // If the number is already divisible by 4096, return it
    if (num % ALIGNMENT == 0) {
        return num;
    }

    // Otherwise, round up to the next multiple of 4096
    return (num + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

int main() {
	uint32_t addr = 0x4000; // 0x307000
	uint32_t test = (addr / 0x1000) % 8; // Bit count
	
	
	printf("Is at pos: 0x%x\n", addr);
	printf("Divided by 4096: %d\n", test);
	//printf("Bit is: %d\n", bytepos % 8);
}
