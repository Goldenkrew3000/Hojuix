#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <kernel.h>
#include <kernel/i386/io.h>
#include <kernel/drivers/pci.h>
#include <kernel/memory/pmmgr.h>

#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

void pci_init() {
    printf("[PCI] Initializing...\n");

    // Allocate a new block to store the PCI Device Table in, and store the address in the global kernel storage
    uint64_t* pci_device_table_addr = (uint64_t*)((uint64_t)pmmgr_kmalloc(1) + ((uint64_t)kerndata.hhdm_offset));
    kerndata.pci_devices_addr = (uint64_t)pci_device_table_addr;
    printf("[PCI] Device Table Addr: %p\n", pci_device_table_addr);

    uint16_t vendor;
    uint16_t device;
    uint16_t class_id;
    int index = 0;

    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 32; j++) {
            for (int k = 0; k < 8; k++) {
                vendor = pci_readWord(i, j, k, 0x0);
                device = pci_readWord(i, j, k, 0x2);
                class_id = pci_readWord(i, j, k, 0xA);
                if (vendor != 0xFFFF) { // Check to see if a PCI device exists
                    // Store the device in the PCI Device Table
                    struct t_pci_device pci_device;
                    pci_device.vendor_id = vendor;
                    pci_device.device_id = device;
                    pci_device.class_id = class_id;
                    pci_device.bus = i;
                    pci_device.slot = j;
                    pci_device.function = k;

                    // Memcpy the data from the struct to the allocated block, with the correct index offset (Struct is 72 bits, or 9 bytes), and iterate the index
                    memcpy((uint64_t*)((uint64_t)pci_device_table_addr + (uint64_t)(0x9 * index)), &pci_device, sizeof(struct t_pci_device));
                    index++;
                    
                    printf("PCI Device %04X:", vendor);
                    printf("%04X [", device);
                    printf("%04X] found. (Bus ", class_id);
                    printf("%d, Slot ", i);
                    printf("%d, Function ", j);
                    printf("%d)\n", k);


                    //printf(" - %s (", pci_searchClassCode(class_id));
                    //printf("%s)\n", pci_searchDevices(vendor, device));
                }
            }
        }
    }

    // Store the number of PCI devices found in the global kernel data storage
    kerndata.pci_device_count = index;
}

uint16_t pci_readWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;

    // Create configuration address
    address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    // Write out the address
    out32(PCI_CONFIG_ADDR, address);

    // Read in the data
    tmp = (uint16_t)((in32(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}

char* pci_searchClassCode(uint16_t class) {
    for (int i = 0; pci_classCodes[i].classCode != 0xFFFF; i++) {
        if (pci_classCodes[i].classCode == class) {
            return pci_classCodes[i].className;
        }
    }

    return "Unknown";
}

char* pci_searchDevices(uint16_t vendor, uint16_t device) {
    for (int i = 0; pci_deviceNames[i].vendor != 0xFFFF; i++) {
        if (pci_deviceNames[i].vendor == vendor && pci_deviceNames[i].device == device) {
            return pci_deviceNames[i].deviceName;
        }
    }

    return "Unknown";
}
