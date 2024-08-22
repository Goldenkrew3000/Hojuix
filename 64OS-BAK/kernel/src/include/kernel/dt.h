#ifndef _DT_H
#define _DT_H

typedef struct gdt_entry_t {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base;
    uint8_t access;
    uint8_t base_high;
    uint8_t flags;
} __attribute__((packed)) gdt_entry_t;


struct gdt_ptr_s {
    uint16_t limit;
    uint32_t base;
} __attribute((packed));


struct idt_entry_s {
    uint16_t base_lo;
    uint16_t sel;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_hi;
} __attribute__((packed));

struct idt_ptr_s {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// TSS HERE TODO

struct isr_registers_s {
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_no;
    unsigned int eip, cs, eflags, useresp, ss;
};

void idt_init();

#endif