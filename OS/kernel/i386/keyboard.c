#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <kernel/io.h>
#include <kernel/dt.h>
#include <kernel/keyboard.h>

typedef struct {
    uint8_t scancode;
    char chr;
} kbd_scancodes;

// Scancodes obtained from https://wiki.osdev.org/PS/2_Keyboard
static kbd_scancodes layer0_scancodes[] = {
    {0x02, '1'},
    {0x03, '2'},
    {0x04, '3'},
    {0x05, '4'},
    {0x06, '5'},
    {0x07, '6'},
    {0x08, '7'},
    {0x09, '8'},
    {0x0A, '9'},
    {0x0B, '0'},
    {0x0C, '-'},
    {0x0D, '='},
    {0x0E, '?'}, // Backspace
    {0x0F, '?'}, // Tab
    {0x10, 'q'},
    {0x11, 'w'},
    {0x12, 'e'},
    {0x13, 'r'},
    {0x14, 't'},
    {0x15, 'y'},
    {0x16, 'u'},
    {0x17, 'i'},
    {0x18, 'o'},
    {0x19, 'p'},
    {0x1A, '['},
    {0x1B, ']'},
    {0x1C, '\n'}, // Enter
    {0x1E, 'a'},
    {0x1F, 's'},
    {0x20, 'd'},
    {0x21, 'f'},
    {0x22, 'g'},
    {0x23, 'h'},
    {0x24, 'j'},
    {0x25, 'k'},
    {0x26, 'l'},
    {0x27, ';'},
    {0x28, '\''},
    {0x29, '`'},
    {0x2B, '\\'},
    {0x2C, 'z'},
    {0x2D, 'x'},
    {0x2E, 'c'},
    {0x2F, 'v'},
    {0x30, 'b'},
    {0x31, 'n'},
    {0x32, 'm'},
    {0x33, ','},
    {0x34, '.'},
    {0x35, '/'},
    {0x39, ' '}
};

static kbd_scancodes layer1_scancodes[] = {
    {0x02, '!'},
    {0x03, '@'},
    {0x04, '#'},
    {0x05, '$'},
    {0x06, '%'},
    {0x07, '^'},
    {0x08, '&'},
    {0x09, '*'},
    {0x0A, '('},
    {0x0B, ')'},
    {0x0C, '_'},
    {0x0D, '+'},
    {0x0E, '?'}, // Backspace
    {0x0F, '?'}, // Tab
    {0x10, 'Q'},
    {0x11, 'W'},
    {0x12, 'E'},
    {0x13, 'R'},
    {0x14, 'T'},
    {0x15, 'Y'},
    {0x16, 'U'},
    {0x17, 'I'},
    {0x18, 'O'},
    {0x19, 'P'},
    {0x1A, '{'},
    {0x1B, '}'},
    {0x1C, '\n'}, // Enter
    {0x1E, 'A'},
    {0x1F, 'S'},
    {0x20, 'D'},
    {0x21, 'F'},
    {0x22, 'G'},
    {0x23, 'H'},
    {0x24, 'J'},
    {0x25, 'K'},
    {0x26, 'L'},
    {0x27, ':'},
    {0x28, '"'},
    {0x29, '~'},
    {0x2B, '|'},
    {0x2C, 'Z'},
    {0x2D, 'X'},
    {0x2E, 'C'},
    {0x2F, 'V'},
    {0x30, 'B'},
    {0x31, 'N'},
    {0x32, 'M'},
    {0x33, '<'},
    {0x34, '>'},
    {0x35, '?'}, // Actually a question mark
    {0x39, ' '}
};

static kbd_scancodes layer2_scancodes[] = {
    {0x02, '1'},
    {0x03, '2'},
    {0x04, '3'},
    {0x05, '4'},
    {0x06, '5'},
    {0x07, '6'},
    {0x08, '7'},
    {0x09, '8'},
    {0x0A, '9'},
    {0x0B, '0'},
    {0x0C, '-'},
    {0x0D, '='},
    {0x0E, '?'}, // Backspace
    {0x0F, '?'}, // Tab
    {0x10, 'Q'},
    {0x11, 'W'},
    {0x12, 'E'},
    {0x13, 'R'},
    {0x14, 'T'},
    {0x15, 'Y'},
    {0x16, 'U'},
    {0x17, 'I'},
    {0x18, 'O'},
    {0x19, 'P'},
    {0x1A, '['},
    {0x1B, ']'},
    {0x1C, '\n'}, // Enter
    {0x1E, 'A'},
    {0x1F, 'S'},
    {0x20, 'D'},
    {0x21, 'F'},
    {0x22, 'G'},
    {0x23, 'H'},
    {0x24, 'J'},
    {0x25, 'K'},
    {0x26, 'L'},
    {0x27, ';'},
    {0x28, '\''},
    {0x29, '`'},
    {0x2B, '\\'},
    {0x2C, 'Z'},
    {0x2D, 'X'},
    {0x2E, 'C'},
    {0x2F, 'V'},
    {0x30, 'B'},
    {0x31, 'N'},
    {0x32, 'M'},
    {0x33, ','},
    {0x34, '.'},
    {0x35, '/'},
    {0x39, ' '}
};

static char keyboard_buffer[256] = {};
static uint8_t keyboard_buffer_position = 0;
bool lshift_toggle = false;
bool rshift_toggle = false;
bool capslock_toggle = false;
bool scrolllock_toggle = false;
bool numlock_toggle = false;
uint8_t ledbyte = 0x00;

void keyboard_buffer_push(char value) {
    if (keyboard_buffer_position != 255) {
        keyboard_buffer[keyboard_buffer_position++] = value;
    } else {
        printf("Keyboard buffer overflow!\n");
        abort();
    }
}

char keyboard_buffer_pop() {
    if (keyboard_buffer_position > 0) {
        return keyboard_buffer[--keyboard_buffer_position];
    } else {
        return 0;
    }
}

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

void keyboard_handler(struct regs* r) {
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

    
    else if (scancode == 0xE0) {
        // Special Key Handler
        
        while(inb(0x64) & 0x02) {
            asm volatile("nop");
        }

        uint8_t special_scancode = inb(0x60);

        if (special_scancode == 0x4B) {
            printf("Left Arrow pressed\n");
        }
    }
    

    //keyboard_buffer_push(keyboard_scancode_conv(scancode));
    printf("%c", keyboard_scancode_conv(scancode));
}

void keyboard_install(void) {
    irq_install_handler(1, keyboard_handler);
}









// TODO WAIT FOR KEYBOARD ACK
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

void keyboard_wait() {
    // EXPERIMENTAL Wait for keyboard controller
    
    uint8_t status = inb(0x64);
    printf("Status: 0x%x\n", status);
}