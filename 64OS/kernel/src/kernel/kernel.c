#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h> // Note to self: DO NOT REMOVE STRING.H, Clang can call memcpy, memset, memmov, and memcmp automatically, hence they are needed
#include <kernel_ext/limine.h>
#include <kernel/framebuffer.h>

extern void krnl_halt();

// Set the limine base revision to 2 (Latest)
__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

// Setup the limine framebuffer
__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Define the start and end markers for the limine requests
__attribute__((used, section(".request_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

void kernel_fill_rectangle(struct limine_framebuffer *fb, int width, int height, uint32_t color) {
    // Non functional, oppo style
    volatile uint32_t *fb_ptr = fb->address;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            fb_ptr[j] = color;
        }
        fb_ptr += fb->pitch;
    }
}

void kernel_entry(void) {
    // KERNEL ENTRY POINT

    // Ensure limine understands our base revision
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        krnl_halt();
    }

    //print_ok();

    // Ensure we have got a framebuffer
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        krnl_halt();
    }

    // Fetch the first framebuffer
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    
    

    //uint64_t fb_width = fb->width;


    // NOTE: Assume the framebuffer model is RGB with 32bit pixels
    
    //kernel_draw_pixel(framebuffer, 0, 0, 0xFF0000);
    framebuffer_fill_background(framebuffer, 0x3F4452);

    framebuffer_ssfn_init(framebuffer);
    //framebuffer_putstring("Hello World!");
    int num = 10;
    printf("Hi\n");
    printf("aNd aNoThEr %d\n", num);

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
    


    //hcf();
    krnl_halt();
}

/*
for (size_t i = 0; i < 512; i++) {
        volatile uint32_t *fb_ptr = fb->address;
        fb_ptr[i * (fb->pitch / 4) + i] = 0xff0000;
        // 0xFF0000 <-- RRGGBB Hex code

    }*/