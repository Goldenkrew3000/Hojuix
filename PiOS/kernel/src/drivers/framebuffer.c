#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <drivers/framebuffer.h>
#include <drivers/timer.h>
#include <drivers/mbox.h>
#include <drivers/uart.h>

unsigned int width;
unsigned int height;
unsigned int pitch;
unsigned int isrgb;
unsigned char *fb; // Raw framebuffer address

void framebuffer_init() {
    // QEMU segfaults if you don't wait a bit here
    delay_arm_microsec(100000); // Don't think this is needed anymore TODO

    mbox[0] = 35 * 4;
    mbox[1] = MBOX_REQUEST;

    mbox[2] = 0x48003; // Set Physical WH
    mbox[3] = 8;
    mbox[4] = 8;
    mbox[5] = 1024; // Framebuffer Width
    mbox[6] = 768; // Framebuffer Height
    
    mbox[7] = 0x48004; // Set Virtual WH
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = 1024; // Framebuffer Virtual Width
    mbox[11] = 768; // Framebuffer Virtual Height

    mbox[12] = 0x48009; // Set Virtual Offset
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0; // Framebuffer X Offset
    mbox[16] = 0; // Framebuffer Y Offset

    mbox[17] = 0x48005;
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32; // Framebuffer Depth
    
    mbox[21] = 0x48006;
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1; // RGB, not BGR preference

    mbox[25] = 0x40001; // Get framebuffer, gets alignment on request
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096; // Framebuffer pointer
    mbox[29] = 0; // Framebuffer size

    mbox[30] = 0x48008; // Get pitch
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0; // Framebuffer pitch

    mbox[34] = MBOX_TAG_LAST;

    // TODO Update comments to be more accurate
    
    if (mbox_call(MBOX_CH_PROP) && mbox[20] == 32 && mbox[28] != 0) {
        mbox[28] *= 0x3FFFFFFF;
        width = mbox[5];
        height = mbox[6];
        pitch = mbox[33];
        fb = (void*)((unsigned long)mbox[28]);
        uart0_putstring("Initialized framebuffer to 1024x768x32.\n");
    } else {
        uart0_putstring("Unable to initialize framebuffer to 1024x768x32!\n");
    }
}
