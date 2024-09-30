#ifndef _DT_H
#define _DT_H

/*
// GDT
*/

/*
struct GDTEntry {
    uint16_t limit1;
    uint16_t base1;
    uint8_t base2;
    uint8_t accessByte;
    uint8_t limit2 : 4;
    uint8_t flags : 4;
    uint8_t base3;
} __attribute__((packed));

//struct gdt_t {

//} __attribute__((packed));

struct GDTPtr {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed));
*/
struct GDTEntry gdt_assembleEntry(uint8_t accessByte, uint8_t flags, uint32_t limit);
void gdt_setGate(int gateID, uint8_t accessByte, uint8_t flags, uint32_t limit);
void loadGDT();
void gdt_init();

/*
// IDT
*/

struct IDTEntry {
    uint16_t offset1;
    uint16_t segmentSelector;
    uint8_t ist : 3;
    uint8_t reserved0 : 5;
    uint8_t gateType : 4;
    uint8_t empty : 1;
    uint8_t dpl : 2;
    uint8_t present : 1;
    uint16_t offset2;
    uint32_t offset3;
    uint32_t reserved;
} __attribute__((packed));

struct IDTPtr {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed));

void idt_setDescriptor(uint8_t vect, void* isrThingy, uint8_t gateType, uint8_t dpl);
void idt_init();

/*
// IRQ
*/

void irq_handler(int int_no);
void irq_remap();
void irq_init();
void irq_unknown_handler(void*);

#endif
