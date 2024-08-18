#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <kernel/tty.h>
#include <kernel/dt.h>
#include <kernel/util.h>
#include <kernel/timer.h>
#include <kernel/io.h>
#include <kernel/keyboard.h>
#include <kernel_ext/multiboot.h>

//#include <stdio.h>
#include <stdlib.h>


// TESTING

#include <string.h>
#include <cpuid.h>




void putpixel(int pos_x, int pos_y, unsigned char VGA_COLOR)
{
    unsigned char* location = (unsigned char*)0xA0000 + 320 * pos_y + pos_x;
    *location = VGA_COLOR;
}




uint8_t kbdus[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

void kernel_main(multiboot_info_t* mbd, uint32_t magic) {
    terminal_initialize();

    printf("PREBOOT INIT\n");
    
    // Initialize GDT
    printf("Initializing GDT...");
    gdt_init();
    printf(" OK\n");

    // Initialize IDT
    printf("Initializing IDT...");
    idt_init();
    printf(" OK\n");

    // Initialize IRQ
    printf("Initializing IRQ...");
    irq_init();
    printf(" OK\n");

    // Allow Interrupts
    asm volatile("sti");

    // Enable Cursor
    // Apparently if GRUB timeout is set to 0, GRUB wont enable the cursor.
    // So it is always good practice to enable it anyway.
    enable_cursor();

    // Enable Keyboard
    keyboard_install();

    // Enable PIT Timer
    pit_timer_install();

    // Check GRUB Multiboot
    multiboot_handler(mbd, magic);

    printf("PREBOOT INIT DONE\n");

    // Kernel Start
    printf("Welcome to Hojuix!\n");
    printf("(C) Goldenkrew3000 2024.\n");
    


    printf("Testing\n");
    //outb(0x60, 0xED);
    //outb(0x60, 0x00 | (1 << 2));
    //uint8_t r_scancode = inb(0x60);
    //printf("Rec: 0x%x\n", r_scancode);

    /*
    while (1) {
        uint8_t scancode = inb(0x60);
        if (scancode == 0x3A) {
            printf("Capslock Triggered\n");

            keyboard_capslock();
        } else if (scancode == 0x46) {
            printf("Scroll lock Triggered\n");

            keyboard_scrolllock();
        } else if (scancode == 0x45) {
            printf("Numlock triggered\n");

            keyboard_numlock();
        }
    }*/

    uint8_t inp = inb(0x64);
    printf("KEY: 0x%x\n", inp);

    uint8_t good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    printf("Good?\n");

    //printf("Waiting 5\n");
    //timer_wait(5000);
    //reboot(); // DOES NOT REBOOT ON XEON BOARD




      unsigned int eax, ebx, ecx, edx;
    char brand[0x40]; // 64 bytes to hold the brand string

    // Get the CPU brand string
    __cpuid(0x80000002, eax, ebx, ecx, edx);
    memcpy(brand, &eax, 4);
    memcpy(brand + 4, &ebx, 4);
    memcpy(brand + 8, &ecx, 4);
    memcpy(brand + 12, &edx, 4);

    __cpuid(0x80000003, eax, ebx, ecx, edx);
    memcpy(brand + 16, &eax, 4);
    memcpy(brand + 20, &ebx, 4);
    memcpy(brand + 24, &ecx, 4);
    memcpy(brand + 28, &edx, 4);

    __cpuid(0x80000004, eax, ebx, ecx, edx);
    memcpy(brand + 32, &eax, 4);
    memcpy(brand + 36, &ebx, 4);
    memcpy(brand + 40, &ecx, 4);
    memcpy(brand + 44, &edx, 4);

    // Ensure the string is null-terminated
    brand[48] = '\0';

    // Print the CPU brand string
    printf("CPU Brand: %s\n", brand);


    while(1) { }


    // TODO
    // Get arrow keys (E0 keys) working in keyboard function
    // CPUID Stuff
    // MEMORY MANAGEMENT (Probably paging)
    // A basic shell

    // 미래
    // Fix general exception handler
    // Basic sound?
}
