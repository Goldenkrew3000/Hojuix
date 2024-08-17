#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
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

void move_cursor(int row, int column) {
    unsigned cursor_pos = row * 80 * column;
    outb(0x3D4, 14);
    outb(0x3D5, cursor_pos >> 8);
    outb(0x3D5, 15);
    outb(0x3D5, cursor_pos);
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

    int i;
    for (i = 0; i < mbd->mmap_length; i += sizeof(multiboot_memory_map_t)) {
        multiboot_memory_map_t* mmmt = (multiboot_memory_map_t*) (mbd->mmap_addr + i);
        uint32_t start_addr = mmmt->addr;
        uint64_t length = mmmt->len;
        uint64_t size = mmmt->size;
        uint32_t type = mmmt->type;
        printf("Start Address: 0x%x ", start_addr);
        printf("Length: 0x%x ", length);
        printf("Size: 0x%x ", size);
        printf("Type: 0x%d\n", type);
    }
}
