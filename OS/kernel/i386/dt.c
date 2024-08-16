#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct gdt_entry {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed));

struct gdt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute((packed));

struct gdt_entry gdt[3];
struct gdt_ptr gdtp;

struct idt_entry {
    unsigned short base_lo;
    unsigned short sel;
    unsigned char always0;
    unsigned char flags;
    unsigned short base_hi;
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char granularity) {
    // Setup the descriptor base address
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    // Setup the descriptor limits
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    // Setup the granularity and access flags
    gdt[num].granularity |= (granularity & 0xF0);
    gdt[num].access = access;
}

// GRUB Sets up a GDT but it is ONLY for booting
// If you modify it, it _apparently_ causes a triple fault.
// So you have to create a new GDT, load it, and far jump (All within Protected Mode)
// P.S. You don't have to set the PE Bit here (Already set by GRUB)

void gdt_init() {
    // Setup the GDT pointer and limit
    gdtp.limit = (sizeof(struct gdt_entry) * 3) - 1; // 3 Segments
    gdtp.base = (uint32_t)&gdt;

    // Null descriptor
    gdt_set_gate(0, 0, 0, 0, 0);

    // Code segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // Data segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // Load new GDT, and Far Jump
    asm volatile("lgdtl %0         \n\t"
                 "ljmp $0x08,$.L%= \n\t"
                 ".L%=:            \n\t"
                 "mov %1, %%ds     \n\t"
                 "mov %1, %%es     \n\t"
                 "mov %1, %%fs     \n\t"
                 "mov %1, %%gs     \n\t"
                 "mov %1, %%ss     \n\t" :: "m"(gdtp), "r"(0x10) );
}

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags) {
    idt[num].base_lo = base & 0xFFFF;
    idt[num].base_hi = (base >> 16) & 0xFFFF;

    idt[num].sel = sel;
    idt[num].always0 = 0;

    idt[num].flags = flags | 0x60;
}

void idt_init() {
    // Setup the special IDT pointer
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    // Completely clear the IDT
    memset(&idt, 0, sizeof(struct idt_entry) * 256);

    //
    //
    // Load the IDT
    uint32_t ptr = (uint32_t) &idtp;
    asm volatile("movl %0,%%eax \n\t"
                 "lidt (%%eax)" : : "r" (ptr) : "eax");
}
