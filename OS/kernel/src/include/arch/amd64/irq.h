#ifndef _ISR_H
#define _ISR_H

#define IRQ_PIT 0
#define IRQ_PS2_KBD 1
// IRQ 2 is never used by the kernel
#define IRQ_TTYS1 3
#define IRQ_TTYS0 4
// IRQ 5 is for LPT2
// IRQ 6 is for a Floppy Drive (Idk, I might implement it because you technically could have a floppy drive on a UEFI system)
// IRQ 7 is for LPT1
#define IRQ_CMOS 8

void irq_init();
void irq_configure();
void irq_mask(int irq_number);
void irq_unmask(int irq_number);
void irq_ack(int irq_number);

#endif
