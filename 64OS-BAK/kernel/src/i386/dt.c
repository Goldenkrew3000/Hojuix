#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <kernel/dt.h>

//#define GDT_ENTRIES 5
//gdt_entry_t gdt[GDT_ENTRIES];

// Created using the tool in the GDT Tutorial at the bottom of the page on the OSDev Wiki
uint64_t gdt[5] = {
    0x0000000000000000,
    0x00AF9A000000FFFF,
    0x00CF92000000FFFF,
    0x00AFFA000000FFFF,
    0x00CFF2000000FFFF
};


//struct gdt_ptr_s gdtp;

struct idt_entry_s idt[256];
struct idt_ptr_s idtp;

void gdt_set_gate(gdt_entry_t* gdt, uint32_t base, uint8_t flags, uint8_t access, uint16_t limit) {
    gdt->base = base >> 24;
    gdt->base_high = base >> 16 & 0xFF;
    gdt->base_low = base & 0xFFFF;
    gdt->flags = flags;
    gdt->access = access;
    gdt->limit_low = limit;
}

struct GDTPtr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));
struct GDTPtr gdt_ptr;

/*
void tss_init(uint32_t idx, uint16_t ss0, uint32_t esp0) {
    // Create the base and limit of the TSS entry into the GDT
    uint32_t base = (uint32_t)&tss_entry;
    uint32_t limit = base + sizeof(tss_entry);

    // Add the TSS descriptor's address to the GDT
    gdt_set_gate(idx, base, limit, 0x89, 0x0);

    // Make sure the descriptor is initially zero
    memset(&tss_entry, 0, sizeof(tss_entry));

    // Set the kernel's stack segment and pointer
    tss_entry.ss0 = ss0;
    tss_entry.esp0 = esp0;

    // Set the CS, SS, DS, ES, FS and GS entries in the TSS.
    // WAY more info here: https://github.com/h5n1xp/CuriOS/blob/main/SourceCode/descriptor_tables.c
    tss_entry.cs = 0x1b;
    tss_entry.ss = 0x13;
    tss_entry.ds = 0x13;
    tss_entry.es = 0x13;
    tss_entry.fs = 0x13;
    tss_entry.gs = 0x23;
}
*/

void gdt_init() {
    // Null descriptor
    //gdt_set_gate(&gdt[0], 0, 0, 0, 0);
    //gdt[0] = 0x00000000;

    // Setup the IDT pointer
    //gdtp.limit = (sizeof(struct gdt_ptr_s) * 5) - 1;
    //gdtp.base = (uint32_t)&gdt;

    // Kernel Code segment
    //gdt_set_gate(&gdt[1], 0, 0xAF, 0x9A, 0xFFFF);

    // Kernel Data segment
    //gdt_set_gate(&gdt[2], 0, 0xCF, 0x92, 0xFFFF);

    // User mode Code segment
    //gdt_set_gate(&gdt[3], 0, 0xAF, 0xFA, 0xFFFF);

    // User mode Data segment
    //gdt_set_gate(&gdt[4], 0, 0xCF, 0xF2, 0xFFFF);

    // Load the GDT
    //printf("GDT Addr: 0x%.16llX\n", gdt);
    //load_gdt((uint32_t)gdt);

    

    gdt_ptr.limit = (sizeof(gdt) - 1);
    gdt_ptr.base = (uint64_t)&gdt;
    

    printf("GDT Addr: 0x%.16llX\n", &gdt_ptr);
    load_gdt((uint32_t)&gdt_ptr); // 
    
    

    // TSS
    //tss_init(5, 0x10, stack_top); // TODO

    // Load new GDT
    //load_gdt((uint32_t)&gdtp);

    // Load the TSS TODO
    // NOTE: The TR register only takes on a new value after loading the TSS as well after the GDT
    // NOTE: I think loading the TSS like this is only required for Software Multitasking
    //load_tss();
}










/*
// IDT / ISR
*/

void idt_init() {
    // Setup the IDT pointer
    idtp.limit = (sizeof(struct idt_entry_s) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    // Completely clear the IDT
    //memset(&idt, 0, sizeof(struct idt_entry_s) * 256);
}