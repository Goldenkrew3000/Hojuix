#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <kernel/util.h>
#include <kernel/io.h>
#include <kernel_ext/multiboot.h>
#include <stdio.h>
#include <stdlib.h>

void enable_cursor() {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | 0);
    outb(0x3D5, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | 15);
}

void move_cursor(uint32_t row, uint32_t column) {
    unsigned cursor_pos = row * 80 + column;
    outb(0x3D4, 14);
    outb(0x3D5, cursor_pos >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, cursor_pos);
}

void reboot() {
    outb(0x64, 0xFE);
}

void multiboot_handler(multiboot_info_t* mbd, uint32_t magic) {
    
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        printf("Invalid boot magic.\n");
        abort();
    } else {
        printf("Verified boot magic.\n");
    }

    if (!(mbd->flags >> 6 &0x1)) {
        printf("Invalid memory map.\n");
        abort();
    } else {
        printf("Verified memory map.\n");
    }

    long long usableMemory = 0;
    long reservedMemory = 0;

    int i;
    for (i = 0; i < mbd->mmap_length; i += sizeof(multiboot_memory_map_t)) {
        multiboot_memory_map_t* mmmt = (multiboot_memory_map_t*) (mbd->mmap_addr + i);
        uint64_t start_addr = mmmt->addr;
        uint64_t length = mmmt->len;
        uint32_t size = mmmt->size;
        uint32_t type = mmmt->type;

        if (type == 1) {
            // Type 1 is usable memory
            usableMemory += length;
        } else if (type == 2) {
            // Type 2 is reserved memory
            reservedMemory += length;
        }

        printf("Start Address: 0x%x ", start_addr);
        printf("Length: 0x%x ", length);
        printf("Size: 0x%x ", size);
        printf("Type: 0x%d\n", type);
    }

    long long usableMemoryMb = usableMemory / 1024 / 1024;
    long reservedMemoryMb = reservedMemory / 1024 / 1024;

    printf("Usable memory: 0x%x (", usableMemory);
    printf("%lld) bytes or ", usableMemory);
    printf("%d mb.\n", usableMemoryMb);
    printf("Reserved memory: 0x%x (", reservedMemory);
    printf("%ld) bytes or ", reservedMemory);
    printf("%d mb.\n", reservedMemoryMb);
}
