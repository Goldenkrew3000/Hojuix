#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <kernel/tty.h>
#include <kernel/dt.h>
#include <kernel_ext/multiboot.h>

#include <stdio.h>
#include <stdlib.h>

// TESTING
#include <kernel/io.h>
#include <string.h>

 
 /* reverse:  reverse string s in place */
 void reverse(char s[])
 {
     int i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
 }

 /* itoa:  convert n to characters in s */
 void itoa(int n, char s[])
 {
     int i, sign;
 
     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
 }

void kernel_main(multiboot_info_t* mbd, uint32_t magic) {
    terminal_initialize();
    printf("PREBOOT INIT\n");
    
    // Initialize new GDT
    printf("Initializing new GDT...\n");
    gdt_init();

    // Initialize IDT
    printf("Initializing IDT...\n");
    idt_init();

    // Check GRUB Multiboot

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        printf("Invalid Boot Magic!\n");
    } else {
        printf("Verified Boot Magic.\n");
    }

    if (!(mbd->flags >> 6 &0x1)) {
        printf("Invalid memory map from GRUB!\n");
    } else {
        printf("Memory map verified!\n");
    }

/*
    int i;
    for (i = 0; i < mbd->mmap_length; i += sizeof(multiboot_memory_map_t)) {
        multiboot_memory_map_t* mmmt = (multiboot_memory_map_t*) (mbd->mmap_addr + i);

        char addr[16];
        char len[16];
        char size[16];
        char type[16];
        itoa(mmmt->addr, addr);
        itoa(mmmt->len, len);
        printf("Start Addr: %s, Length: %s bytes\n", addr, len);
    }
*/

    printf("Welcome to *** OS!\n");
    printf("(C) Goldenkrew3000 2024.\n");

    /*
    unsigned temp;
    temp = 7*80+7;
    outb(0x3D4, 14);
    outb(0x3D5, temp >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, temp);*/


    while(1) { }
}
