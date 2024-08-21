#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <kernel/tty.h>
#include <kernel/vga.h>
#include <kernel/util.h>
#include <string.h>

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_BLACK, VGA_L_GREY);
    //terminal_color = vga_entry_color(VGA_WHITE, VGA_BLACK);
    terminal_buffer = VGA_MEMORY;
    
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_putentryat(char character, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(character, color);
}

void terminal_putchar(char c) {
    unsigned char uc = c;

    if (c == '\n') {
        ++terminal_row;
        terminal_column = 0;
    } else {
        terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);
        terminal_column++;
    }

    if (terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }

    if (terminal_row == VGA_HEIGHT) {
        terminal_scroll();
        terminal_row = VGA_HEIGHT - 1;
    }

    move_cursor(terminal_row, terminal_column);
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

void terminal_scroll() {
    // Move up one line
    memmove(&terminal_buffer[0], &terminal_buffer[VGA_WIDTH], sizeof(uint16_t) * VGA_WIDTH * (VGA_HEIGHT - 1));

    // Set the memory space to 0
    memset(&terminal_buffer[VGA_WIDTH * (VGA_HEIGHT - 1)], 0, sizeof(uint16_t) * VGA_WIDTH);

    // Clear new line with proper color
    uint8_t color = vga_entry_color(VGA_BLACK, VGA_L_GREY); // 0 for black, 1 for blue
    uint16_t entry = vga_entry(' ', color);
    for (size_t i = 0; i < VGA_WIDTH; ++i) {
        terminal_buffer[VGA_WIDTH * (VGA_HEIGHT - 1) + i] = entry;
    }
}