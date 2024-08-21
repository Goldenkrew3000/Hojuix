#ifndef VGA_H_INCLUDED
#define VGA_H_INCLUDED

enum vga_color {
    VGA_BLACK = 0,
    VGA_BLUE = 1,
    VGA_GREEN = 2,
    VGA_CYAN = 3,
    VGA_RED = 4,
    VGA_MAGENTA = 5,
    VGA_BROWN = 6,
    VGA_L_GREY = 7,
    VGA_D_GREY = 8,
    VGA_L_BLUE = 9,
    VGA_L_GREEN = 10,
    VGA_L_CYAN = 11,
    VGA_L_RED = 12,
    VGA_L_MAGENTA = 13,
    VGA_L_BROWN = 14,
    VGA_WHITE = 15
};

static inline uint8_t vga_entry_color(enum vga_color foreground, enum vga_color background) {
    return foreground | background << 4;
}

static inline uint16_t vga_entry(unsigned char character, uint8_t color) {
    return (uint16_t)character | (uint16_t)color << 8;
}

#endif
