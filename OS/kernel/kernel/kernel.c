#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <kernel/tty.h>
#include <kernel/dt.h>
#include <kernel/util.h>
#include <kernel/timer.h>
#include <kernel/io.h>
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

    // Check GRUB Multiboot
    multiboot_handler(mbd, magic);

    printf("PREBOOT INIT DONE\n");

    pit_timer_install();

    // Kernel Start

    printf("Welcome to ***!\n");
    printf("(C) Goldenkrew3000 2024.\n");

    
    //move_cursor(7, 5);

    printf("Waiting 5 seconds\n");

    //keyborad_install();
    //timer_wait(5000);

    keyboard_install();

    printf("After\n");

    while(1) { }


    // TODO
    // A proper keyboard function
    // A basic shell

    // 미래
    // Fix general exception handler
    // Basic sound?
}
