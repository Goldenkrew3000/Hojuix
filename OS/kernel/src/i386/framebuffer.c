#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <kernel/framebuffer.h>
#include <kernel_ext/limine.h>

#define SSFN_CONSOLEBITMAP_TRUECOLOR // Simple implementation
#include <kernel_ext/ssfn.h> // SSFN2 (https://gitlab.com/bztsrc/scalable-font2)

struct limine_framebuffer *g_fb;
int currentLine = 0;
uint32_t currentColor;

// External linked fonts
extern char _binary_unifont_sfn_start;

/*
// To calculate the character lines on the framebuffer
// Calculate the total framebuffer size: long total_size = fb->pitch * fb->height
// Divide it by the height of a character (16 pixels): total_size / 16
// ABS the value so that if it's a decimal, it always rounds down (So the bottom char is not off screen)
// P.S. ABS is probably not necessary due to common screen sizes but it's there as a precaution
*/

void framebuffer_draw_pixel(int x, int y, uint32_t color) {
    volatile uint32_t *fb_ptr = g_fb->address;
    size_t fb_idx = y * (g_fb->pitch / sizeof(uint32_t)) + x;
    fb_ptr[fb_idx] = color;
}

void framebuffer_fill_background(uint32_t color) {
    currentColor = color;
    for (int y = 0; y < g_fb->height; y++) {
        for (int x = 0; x < g_fb->width; x++) {
            framebuffer_draw_pixel(x, y, color);
        }
    }
}

void framebuffer_scroll(int lines) {
    // TODO: The math is fucked up in here, 4 lines is 16 lines outside of here.
    volatile uint32_t *fb_ptr = g_fb->address;    

    // How many bytes per line
    int line_size = g_fb->pitch * lines;

    // Total size of the framebuffer in bytes
    long total_size = g_fb->pitch * g_fb->height;

    // Scroll by copying memory upwards
    memcpy(g_fb->address, g_fb->address + line_size, total_size - line_size);
    
    // Fill in background color for new line
    for (int y = g_fb->height - lines; y < g_fb->height; y++) {
        for (int x = 0; x < g_fb->width; x++) {
            framebuffer_draw_pixel(x, y, currentColor);
        }
    }
    
}

void framebuffer_ssfn_init(struct limine_framebuffer *fb) {
    // TODO Currently the simple framebuffer
    // Can only implement the proper one once I have libc의 realloc and free
    ssfn_src = (ssfn_font_t*)&_binary_unifont_sfn_start;
    ssfn_dst.ptr = fb->address;
    ssfn_dst.w = fb->width;
    ssfn_dst.h = fb->height;
    ssfn_dst.p = fb->pitch;
    ssfn_dst.x = 0;
    ssfn_dst.y = 0;
    ssfn_dst.fg = 0xFFFFFF;

    // Move framebuffer struct to this file so it can be called with any function here with ease
    g_fb = fb;
}

void framebuffer_putchar(char character) {
    if (character == '\n') {
        ssfn_dst.x = 0;
        if (ssfn_dst.y + 16 >= (abs(g_fb->height / 16) * 16) ) {
            // The reason for the ABS and /16 checking is for if the screen height is not divisible by 16 (1600x900), the newline would be off screen.
            framebuffer_scroll(16);
        } else {
            ssfn_dst.y += 16;
        }
    } else {
        //if (ssfn_dst.x > (9 * 120)) {
        //    ssfn_dst.x = 0;
        //    ssfn_dst.y += 16;
        //}
        ssfn_putc(character);
    }
}

void framebuffer_backspace() {
    // Move back 9 pixels (8 for the character), and 1 because it doesnt look right otherwise
    ssfn_dst.x -= 9;
}


//Note for future on how to properly load SSFN
// Load SSFN
    /*
    ssfn_putc('h');*/

    /*
    ssfn_t ctx = {0};
    ssfn_buf_t buf = {
        .ptr = framebuffer->address, // Framebuffer address
        .w = framebuffer->width,
        .h = framebuffer->height,
        .p = framebuffer->pitch,
        .x = 0, // X to print text
        .y = 0, // Y to print text
        .fg = 0xFFFFFFFF // Foreground color
    };

    // Add one or more fonts to the context (Fonts must already be in memory)
    ssfn_load(&ctx, _binary_unifont_sfn_start); // MIGHT HAVE TO CHANGE TO DEREFERENCE

    // Select typeface to use
    ssfn_select(&ctx, SSFN_FAMILY_SERIF, NULL, SSFN_STYLE_REGULAR | SSFN_STYLE_UNDERLINE, 64); // IDK

    ssfn_render(&ctx, &buf, "Hello world!");
    */
