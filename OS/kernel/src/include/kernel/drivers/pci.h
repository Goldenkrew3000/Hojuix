#ifndef _PCI_H
#define _PCI_H

void pci_init();
uint16_t pci_readWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

#endif
