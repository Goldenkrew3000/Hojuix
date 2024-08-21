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
#include <kernel/mm.h>
#include <kernel_ext/multiboot.h>

//#include <stdio.h>
#include <stdlib.h>


// TESTING

#include <string.h>
#include <cpuid.h>
#include <kernel/mm.h>






 //Play sound using built-in speaker
 static void play_sound(uint32_t nFrequence) {
 	uint32_t Div;
 	uint8_t tmp;
 
        //Set the PIT to the desired frequency
 	Div = 1193180 / nFrequence;
 	outb(0x43, 0xb6);
 	outb(0x42, (uint8_t) (Div) );
 	outb(0x42, (uint8_t) (Div >> 8));
 
        //And play the sound using the PC speaker
 	tmp = inb(0x61);
  	if (tmp != (tmp | 3)) {
 		outb(0x61, tmp | 3);
 	}
 }
 
 //make it shut up
 static void nosound() {
 	uint8_t tmp = inb(0x61) & 0xFC;
     
 	outb(0x61, tmp);
 }
 
 //Make a beep
 void beep() {
 	 play_sound(1000);
 	 timer_wait(500);
 	 nosound();
          //set_PIT_2(old_frequency);
 }






void kernel_main(multiboot_info_t* mbd, uint32_t magic) {
    terminal_initialize();

    // Print compile time to console
    printf("HOJUIX 0.1A - BUILD TIME: %s ", __DATE__);
    printf("%s\n", __TIME__);

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
    //enable_cursor();
    // TODO: Does not work on XEON board. Works with default cursor though

    // Enable Keyboard
    keyboard_install();

    // Enable PIT Timer
    pit_timer_install();

    // Initialize PMM
    init_mmmh(mbd, magic);
    //printf("Regions Init'd: %i, ", pmmgr_get_block_count());
    //printf("Regions Used/Reserved: %i, ", pmmgr_get_use_block_count());
    //printf("Regions Free: %i\n", pmmgr_get_free_block_count());

    printf("PREBOOT INIT DONE\n");

    // Kernel Start
    printf("Welcome to Hojuix!\n");
    printf("(C) Goldenkrew3000 2024.\n");
    
    //reboot();
    // DOES NOT REBOOT ON XEON BOARD Only halts the system. Use ACPI one we have PCI?

    printf("TESTING PMM\n");

    //uint32_t* p = (uint32_t*)pmmgr_alloc_block();
    //printf("P addr: 0x%p\n", &p);

    /*
   
    uint32_t* p2 = (uint32_t*)pmmgr_alloc_blocks(30000);
    uint32_t* p3 = (uint32_t*)pmmgr_alloc_block();
    printf("Allocated new block at 0x%x\n", &p);
    printf("2nd: 0x%x\n", &p2);
    printf("3rd: 0x%x\n", &p3);
    
    printf("P2 addr: 0x%x\n", p2);
    printf("P3 addr: 0x%x\n", p3);
    printf("FREE: %d\n", pmmgr_get_free_block_count());
    //char* haha = "HELLO WORLD!";
    //printf("SIZE: %d\n", strlen(haha));
    //memcpy(&p, haha, strlen(haha));
*/

    while(1) { }


    // TODO
    // Get arrow keys (E0 keys) working in keyboard function
    // CPUID Stuff
    // MEMORY MANAGEMENT (Probably paging) https://wiki.osdev.org/Memory_management
    // Test TSS / Software interrupt 48?
    // A basic shell

    // 미래
    // Fix general exception handler
    // Basic sound? <-- PC Speaker works on XEON board!!

    
}
