#ifndef DT_H_INCLUDED
#define DT_H_INCLUDED

void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char granularity);
void gdt_init();
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);
void idt_init();

#endif
