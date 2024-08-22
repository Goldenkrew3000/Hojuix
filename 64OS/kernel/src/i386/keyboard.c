#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <kernel/dt.h>
#include <kernel/keyboard.h>

bool lshift_toggle = false;
bool rshift_toggle = false;
bool capslock_toggle = false;
bool scrolllock_toggle = false;
bool numlock_toggle = false;
uint8_t ledbyte = 0x00;

char keyboard_scancode_conv(uint8_t scancode) {
    if (lshift_toggle) {
        for (int i = 0; layer1_scancodes[i].scancode != 0x00; i++) {
            if (layer1_scancodes[i].scancode == scancode) {
                return layer1_scancodes[i].chr;
            }
        }
        return 0;
    } else if (rshift_toggle) {
        for (int i = 0; layer1_scancodes[i].scancode != 0x00; i++) {
            if (layer1_scancodes[i].scancode == scancode) {
                return layer1_scancodes[i].chr;
            }
        }
        return 0;
    } else if (capslock_toggle) {
        for (int i = 0; layer2_scancodes[i].scancode != 0x00; i++) {
            if (layer2_scancodes[i].scancode == scancode) {
                return layer2_scancodes[i].chr;
            }
        }
        return 0;
    } else {
        for (int i = 0; layer0_scancodes[i].scancode != 0x00; i++) {
            if (layer0_scancodes[i].scancode == scancode) {
                return layer0_scancodes[i].chr;
            }
        }
        return 0;
    }
}

// PS/2 Keyboard IRQ Handler
__attribute__((interrupt))
void irq_keyboard_handler(void*) {
    unsigned char status = inb(0x64);
    if (status & 0x01) {
        uint8_t scancode = inb(0x60);
        if (scancode == 0x3A) {
            // Capslock pressed
            keyboard_capslock();
        } else if (scancode == 0x46) {
            // Scroll lock pressed
            keyboard_scrolllock();
        } else if (scancode == 0x45) {
            // Numlock pressed
            keyboard_numlock();
        } else if (scancode == 0x2A) {
            // Left shift pressed
            lshift_toggle = true;
        } else if (scancode == 0xAA) {
            // Left shift released
            lshift_toggle = false;
        } else if (scancode == 0x36) {
            // Right shift pressed
            rshift_toggle = true;
        } else if (scancode == 0xB6) {
            // Right shift released
            rshift_toggle = false;
        }
        
        
        else if (scancode == 0x3B) {
            // F1 key pressed
            printf("F1 key pressed\n");
        } else if (scancode == 0x3C) {
            // F2 key pressed
            printf("F2 key pressed\n");
        } else if (scancode == 0x3D) {
            // F3 key pressed
            printf("F3 key pressed\n");
        } else if (scancode == 0x3E) {
            // F4 key pressed
            printf("F4 key pressed\n");
        } else if (scancode == 0x3F) {
            // F5 key pressed
            printf("F5 key pressed\n");
        } else if (scancode == 0x40) {
            // F6 key pressed
            printf("F6 key pressed\n");
        } else if (scancode == 0x41) {
            // F7 key pressed
            printf("F7 key pressed\n");
        } else if (scancode == 0x42) {
            // F8 key pressed
            printf("F8 key pressed\n");
        } else if (scancode == 0x43) {
            // F9 key pressed
            printf("F9 key pressed\n");
        } else if (scancode == 0x44) {
            // F10 key pressed
            printf("F10 key pressed\n");
        } else if (scancode == 0x57) {
            // F11 key pressed
            printf("F11 key pressed\n");
        } else if (scancode == 0x58) {
            // F12 key pressed
            printf("F12 key pressed\n");
        }

        printf("%c", keyboard_scancode_conv(scancode));
    }

    

    

    // ACK the interrupt
    irq_handler(33);
}







void keyboard_capslock() {
    if (capslock_toggle) {
        // Turn off capslock
        capslock_toggle = false;
        
        // Toggle bit 3 (Little endian)
        ledbyte ^= (1 << 2);

        // Toggle capslock light
        outb(0x60, 0xED);
        outb(0x60, ledbyte);
    } else {
        // Turn on capslock
        capslock_toggle = true;

        // Toggle bit 3 (Little endian)
        ledbyte ^= (1 << 2);

        // Turn on capslock light
        outb(0x60, 0xED);
        outb(0x60, ledbyte);
    }
}

void keyboard_scrolllock() {
    if (scrolllock_toggle) {
        // Turn off scroll lock
        scrolllock_toggle = false;

        // Toggle bit 0
        ledbyte ^= (1 << 0);

        // Turn off scroll lock light
        outb(0x60, 0xED);
        outb(0x60, ledbyte);
    } else {
        // Turn on scroll lock
        scrolllock_toggle = true;

        // Toggle bit 0
        ledbyte ^= (1 << 0);

        // Turn on scroll lock light
        outb(0x60, 0xED);
        outb(0x60, ledbyte);
    }
}

void keyboard_numlock() {
    if (numlock_toggle) {
        // Turn off numlock
        numlock_toggle = false;

        // Toggle bit 1
        ledbyte ^= (1 << 1);

        // Turn off numlock light
        outb(0x60, 0xED);
        outb(0x60, ledbyte);
    } else {
        // Turn on numlock
        numlock_toggle = true;

        // Toggle bit 1
        ledbyte ^= (1 << 1);

        // Turn on numlock light
        outb(0x60, 0xED);
        outb(0x60, ledbyte);
    }
}
