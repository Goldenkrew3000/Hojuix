#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <kernel/framebuffer.h>
#include <kernel_ext/limine.h>

#define SSFN_CONSOLEBITMAP_TRUECOLOR // Simple implementation
#include <kernel_ext/ssfn.h> // SSFN2 (https://gitlab.com/bztsrc/scalable-font2)

// External linked fonts
extern char _binary_unifont_sfn_start;

void framebuffer_draw_pixel(struct limine_framebuffer *fb, int x, int y, uint32_t color) {
    // TODO After I can get text from the OS, I can just send the address instead of the whole fb
    // The pitch is needed, but idk what that var looks like yet.
    volatile uint32_t *fb_ptr = fb->address;
    size_t fb_idx = y * (fb->pitch / sizeof(uint32_t)) + x;
    fb_ptr[fb_idx] = color;
}

void framebuffer_fill_background(struct limine_framebuffer *fb, uint32_t color) {
    for (int y = 0; y < fb->height; y++) {
        for (int x = 0; x < fb->width; x++) {
            framebuffer_draw_pixel(fb, x, y, color);
        }
    }
}

void framebuffer_ssfn_init(struct limine_framebuffer *fb) {
    // TODO Currently the simple framebuffer
    // Can only implement the proper one once I have libc의 realloc and free
    ssfn_src = &_binary_unifont_sfn_start;
    ssfn_dst.ptr = fb->address;
    ssfn_dst.w = fb->width;
    ssfn_dst.h = fb->height;
    ssfn_dst.p = fb->pitch;
    ssfn_dst.x = 0;
    ssfn_dst.y = 0;
    ssfn_dst.fg = 0xFFFFFF;
}

void framebuffer_putchar(char character) {
    if (character == '\n') {
        ssfn_dst.x = 0;
        ssfn_dst.y += 16;
    } else {
        ssfn_putc(character);
    }
}

void framebuffer_putstring(char* string) {
    for (int i = 0; i < kern_strlen(string); i++) {
        framebuffer_putchar(string[i]);
    }
    ssfn_dst.x = 0;
    ssfn_dst.y += 16;
}

size_t kern_strlen(char* string) {
    size_t length = 0;
    while (string[length]) {
        length++;
    }
    return length;
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
