#ifndef _ISR_H
#define _ISR_H

void irq_init();
void irq_configure();
void irq_mask(int irq_number);
void irq_unmask(int irq_number);
void irq_ack(int irq_number);

#endif
