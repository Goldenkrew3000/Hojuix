#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <kernel/dt.h>
#include <kernel/io.h>

struct gdt_entry gdt[5];
struct gdt_ptr gdtp;

struct idt_entry idt[256];
struct idt_ptr idtp;

void gdt_set_gate(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    // Setup the descriptor base address
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    // Setup the descriptor limits
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;

    // Setup the granularity and access flags
    gdt[num].granularity |= granularity & 0xF0;
    gdt[num].access = access;
}

// GRUB Sets up a GDT but it is ONLY for booting
// If you modify it, it _apparently_ causes a triple fault.
// So you have to create a new GDT, load it, and far jump (All within Protected Mode)
// P.S. You don't have to set the PE Bit here (Already set by GRUB)

void gdt_init() {
    // Setup the GDT pointer and limit
    gdtp.limit = (sizeof(struct gdt_entry) * 5) - 1; // 3 Segments
    gdtp.base = (uint32_t)&gdt;

    // Null descriptor
    gdt_set_gate(0, 0, 0, 0, 0);

    // Code segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // Data segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    // Load new GDT
    load_gdt((uint32_t)&gdtp);
}

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = base & 0xFFFF;
    idt[num].base_hi = (base >> 16) & 0xFFFF;

    idt[num].sel = sel;
    idt[num].always0 = 0;

    idt[num].flags = flags; // flags | 0x60
}

void idt_init() {
    // Setup the special IDT pointer
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    // Completely clear the IDT
    memset(&idt, 0, sizeof(struct idt_entry) * 256);

    // ISRs
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E);
    idt_set_gate(3, (uint32_t)isr3, 0x08, 0x8E);
    idt_set_gate(4, (uint32_t)isr4, 0x08, 0x8E);
    idt_set_gate(5, (uint32_t)isr5, 0x08, 0x8E);
    idt_set_gate(6, (uint32_t)isr6, 0x08, 0x8E);
    idt_set_gate(7, (uint32_t)isr7, 0x08, 0x8E);
    idt_set_gate(8, (uint32_t)isr8, 0x08, 0x8E);
    idt_set_gate(9, (uint32_t)isr9, 0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);

    // Load the IDT
    load_idt((uint32_t)&idtp);
}

void isr_fault_handler(struct regs *r) {
    if (r->int_no < 32) {
        printf("\n*EXCEPTION %d* ", r->int_no);
        printf("Code %d\n", r->err_no);
        printf("GS: 0x%x, ", r->gs);
        printf("FS: 0x%x, ", r->fs);
        printf("ES: 0x%x, ", r->es);
        printf("DS: 0x%x\n", r->ds);
        printf("EDI: 0x%x, ", r->edi);
        printf("ESI: 0x%x, ", r->esp);
        printf("EBP: 0x%x, ", r->ebp);
        printf("ESP: 0x%x\n", r->esp);
        printf("EBX: 0x%x, ", r->ebx);
        printf("EDX: 0x%x, ", r->edx);
        printf("ECX: 0x%x, ", r->ecx);
        printf("EAX: 0x%x\n", r->eax);
        printf("EIP: 0x%x, ", r->eip);
        printf("CS: 0x%x, ", r->cs);
        printf("EFLAGS: 0x%x, ", r->eflags);
        printf("USERESP: 0x%x, ", r->useresp);
        printf("SS: 0x%x\n", r->ss);
        
        abort();
    }
}

void *irq_routines[16] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

void irq_install_handler(int irq, void (*handler)(struct regs *r)) {
    irq_routines[irq] = handler;
}

void irq_uninstall_handler(int irq) {
    irq_routines[irq] = 0;
}

void irq_remap(void) {
    // Have to remap IRQ 0-7 due to Protected Mode and the IDT
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

void irq_init() {
    irq_remap();

    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2, 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3, 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4, 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5, 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6, 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9, 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);
}

void irq_handler(struct regs* r) {
    // Blank pointer function
    void (*handler)(struct regs *r);

    // Find out if we have a custom handler to run for this IRQ
    handler = irq_routines[r->int_no - 32];
    if (handler) {
        handler(r);
    }

    // If the IDT entry that was invoked was greater than 40 (IRQ 8-15), send an EOI to the slave controller
    if (r->int_no >= 40) {
        outb(0xA0, 0x20);
    }

    // In either case, we need to send an EOI to the master interrupt controller
    outb(0x20, 0x20);
}