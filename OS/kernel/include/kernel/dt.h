#ifndef DT_H_INCLUDED
#define DT_H_INCLUDED

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute((packed));

struct idt_entry {
    uint16_t base_lo;
    uint16_t sel;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_hi;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct tss_entry_struct {
    uint32_t prev_tss; // Used if hardware task switching is used
    uint32_t esp0; // Stack pointer to load when we change to kernel mode
    uint32_t ss0; // Stack segment to load when we change to kernel mode
    uint32_t esp1; // Unused
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es; // The value to load into ES when we change to kernel mode
    uint32_t cs; // Same
    uint32_t ss; // Same
    uint32_t ds; // Same
    uint32_t fs; // Same
    uint32_t gs; // Same
    uint32_t ldt; // Unused
    uint32_t trap;
    uint32_t iomap_base;   
} __attribute__((packed));

typedef struct tss_entry_struct tss_entry_t;

struct regs {
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_no;
    unsigned int eip, cs, eflags, useresp, ss;
};

void gdt_set_gate(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity);
void gdt_init();
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);
void idt_init();
void irq_install_handler(int irq, void (*handler)(struct regs *r));
void irq_uninstall_handler(int irq);
void irq_remap(void);
void irq_init();
void tss_init(uint32_t idx, uint16_t ss0, uint32_t esp0);

// External Loading Functions, held in assembly
extern void load_gdt(uint32_t);
extern void load_idt(uint32_t);
extern void load_tss();

// External ISR Functions, held in assembly
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr48();

// External IRQ Functions, held in assembly
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

#endif