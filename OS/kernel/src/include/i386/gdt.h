#ifndef _GDT_H
#define _GDT_H

struct gdtr_t {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed));

struct tss_t {
    uint32_t rsvd0;
    uint64_t rsp0;
    uint32_t rsvd1[23];
} __attribute__((packed));

void gdt_init();
void tss_init();
uint64_t create_gdt_entry(uint64_t base, uint64_t limit, uint64_t access, uint64_t flags);
void create_system_segment_descriptor(uint64_t *GDT, uint8_t idx, uint64_t base, uint64_t limit, uint64_t access, uint64_t flags);
#endif
