#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <kernel/drivers/pci.h>
#include <kernel/io.h>

#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

void pci_init() {
    printf("Initializing PCI...\n");

    uint16_t vendor;
    uint16_t device;
    uint16_t class_id;

    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 32; j++) {
            for (int k = 0; k < 8; k++) {
                vendor = pci_readWord(i, j, k, 0x0);
                device = pci_readWord(i, j, k, 0x2);
                class_id = pci_readWord(i, j, k, 0xA);
                if (vendor != 0xFFFF) { // Check to see if a PCI device exists
                    printf("PCI Device %04X:", vendor);
                    printf("%04X [", device);
                    printf("%04X] found. (Bus ", class_id);
                    printf("%d, Slot ", i);
                    printf("%d, Function ", j);
                    printf("%d)\n", k);

                    /*
                    if (class_id == 0x0302) {
                        printf("Found GPU!\n");
                    }

                    if (class_id == 0x0108) {
                        printf("Found an NVME Controller\n");
                    }
                    */
                }
            }
        }
    }
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

typedef struct {
    uint16_t classCode;
    char* class;
} pci_classCode;

static pci_classCode class_codes[] = {
    // Unclassified Devices
    {0x0000, "Non-VGA-Compatible Unclassified Device"},
    {0x0001, "VGA-Compatible Unclassified Device"},

    // Mass Storage Controllers
    {0x0100, "SCSI Bus Controller"},
    {0x0101, "IDE Controller"},
    {0x0102, "Floppy Disk Controller"},
    {0x0103, "IPI Bus Controller"},
    {0x0104, "RAID Controller"},
    {0x0105, "ATA Controller"},
    {0x0106, "SATA Controller"},
    {0x0107, "Serial Attached SCSI Controller"},
    {0x0108, "NVMe Controller"},
    {0x0180, "Other Mass Storage Controller"},

    // Network Controllers
    {0x0200, "Ethernet Controller"},
    {0x0201, "Token Ring Controller"},
    {0x0202, "FDDI Controller"},
    {0x0203, "ATM Controller"},
    {0x0204, "ISDN Controller"},
    {0x0205, "WorldFip Controller"},
    {0x0206, "PICMG 2.14 Multi Computing Controller"},
    {0x0207, "Infiniband Controller"},
    {0x0208, "Fabric Controller"},
    {0x0209, "Other Network Controller"},

    // Display Controllers
    // Multimedia Controllers
    // Memory Controllers
    // Bridges
    // Simple Communication Controllers
    // Base System Peripherals
    // Input Device Controllers
    // Docking Stations
    // Processors
    // Serial Bus Controllers
    // Wireless Controllers
    // Intelligent Controllers
    // Satellite Communication Controllers
    // Encryption Controllers
    // Signal Processing Controllers
};
