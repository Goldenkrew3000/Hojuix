#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <kernel/dt.h>
#include <kernel/io.h>
#include <kernel/keyboard.h>
#include <kernel/framebuffer.h>

bool wasSpecialInterrupt = false;

bool lshift_toggle = false;
bool rshift_toggle = false;
bool lctrl_toggle = false;
bool lalt_toggle = false;

bool capslock_toggle = false;
bool scrolllock_toggle = false;
bool numlock_toggle = false;
uint8_t ledbyte = 0x00;

char keyboard_buffer[256];

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

        if (wasSpecialInterrupt) {
            // Key was a special key
            if (scancode == 0xDB) {
                // Left Super Key pressed
                printf("Left Super Key pressed\n");
            } else if (scancode == 0x48) {
                // Arrow Key Up pressed
                printf("Cursor up pressed\n");
            } else if (scancode == 0x50) {
                // Arrow Key Down pressed
                printf("Cursor down pressed\n");
            } else if (scancode == 0x4B) {
                // Arrow Key Left pressed
                printf("Cursor left pressed\n");
            } else if (scancode == 0x4D) {
                // Arrow Key Right pressed
                printf("Cursor right pressed\n");
            } else if (scancode == 0x49) {
                // Page Up pressed
                printf("Page up pressed\n");
            } else if (scancode == 0x51) {
                // Page Down pressed
                printf("Page down pressed\n");
            } else if (scancode == 0x47) {
                // Home pressed
                printf("Home pressed\n");
            } else if (scancode == 0x4F) {
                // End pressed
                printf("End pressed\n");
            } else if (scancode == 0x53) {
                // Delete pressed
                printf("Delete pressed\n");
            }

            wasSpecialInterrupt = false;
        } else {


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
        } else if (scancode == 0x1D) {
            // Left control pressed
            lctrl_toggle = true;
        } else if (scancode == 0x9D) {
            // Left control released
            lctrl_toggle = false;
        } else if (scancode == 0x38) {
            // Left alt pressed
            lalt_toggle = true;
        } else if (scancode == 0xB8) {
            // Left alt released
            lalt_toggle = false;
        } else if (scancode == 0x0E) {
            // Backspace key pressed
            framebuffer_backspace();
        } else if (scancode == 0x76) {
            // Escape key pressed
            printf("Escape key pressed\n");
        } else if (scancode == 0xE0) {
            // Special Key - Set bool for next keycode
            wasSpecialInterrupt = true;
        } else {
            printf("%c", keyboard_scancode_conv(scancode));
        }

        }
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
