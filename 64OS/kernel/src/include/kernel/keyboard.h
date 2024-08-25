#ifndef _KEYBOARD_H
#define _KEYBOARD_H

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
    //{0x0E, '?'}, // Backspace
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
    {0x39, ' '},
    {0x3B, '1'}, // F1
    {0x3C, '2'}, // F2
    {0x3D, '3'}, // F3
    {0x3E, '4'}, // F4
    {0x3F, '5'}, // F5
    {0x40, '6'}, // F6
    {0x41, '7'}, // F7
    {0x42, '8'}, // F8
    {0x43, '9'}, // F9
    {0x44, 'a'}, // F10
    {0x57, 'b'}, // F11
    {0x58, 'c'} // F12
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
    //{0x0E, '?'}, // Backspace
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
    //{0x0E, '?'}, // Backspace
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

char keyboard_scancode_conv(uint8_t scancode);
void keyboard_capslock();
void keyboard_scrolllock();
void keyboard_numlock();
void irq_keyboard_handler(void*);

#endif