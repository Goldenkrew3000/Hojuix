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

char* class_finder(uint16_t class);

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
                    printf("%d)", k);

                    printf(" -- %s\n", class_finder(class_id));

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
    {0x0300, "VGA-Compatible Controller"},
    {0x0301, "XGA Controller"},
    {0x0302, "3D Controller (Non-VGA-Compatible)"},
    {0x0380, "Other Display Controller"},

    // Multimedia Controllers
    {0x0400, "Multimedia Video Controller"},
    {0x0401, "Multimedia Audio Controller"},
    {0x0402, "Computer Telephony Device"},
    {0x0403, "Audio Device"},
    {0x0480, "Other Multimedia Controller"},

    // Memory Controllers
    {0x0500, "RAM Controller"},
    {0x0501, "Flash Controller"},
    {0x0580, "Other Memory Controller"},

    // Bridges
    {0x0600, "Host Bridge"},
    {0x0601, "ISA Bridge"},
    {0x0602, "EISA Bridge"},
    {0x0603, "MCA Bridge"},
    {0x0604, "PCI-to-PCI Bridge"},
    {0x0605, "PCMCIA Bridge"},
    {0x0606, "NuBus Bridge"},
    {0x0607, "CardBus Bridge"},
    {0x0608, "RACEway Bridge"},
    {0x0609, "PCI-to-PCI Bridge"},
    {0x060A, "InfiniBand-to-PCI Host Bridge"},
    {0x0680, "Other Bridge"},

    // Simple Communication Controllers
    {0x0700, "Serial Controller"},
    {0x0701, "Parallel Controller"},
    {0x0702, "Multiport Serial Controller"},
    {0x0703, "Modem"},
    {0x0704, "IEEE 488.1/2 (GPIB) Controller"},
    {0x0705, "Smart Card Controller"},
    {0x0780, "Other Simple Communication Controller"},

    // Base System Peripherals
    {0x0800, "PIC"},
    {0x0801, "DMA Controller"},
    {0x0802, "Timer"},
    {0x0803, "RTC Controller"},
    {0x0804, "PCI Hot-Plug Controller"},
    {0x0805, "SD Host Controller"},
    {0x0806, "IOMMU"},
    {0x0880, "Other Base System Peripheral"},

    // Input Device Controllers
    {0x0900, "Keyboard Controller"},
    {0x0901, "Digitizer Pen"},
    {0x0902, "Mouse Controller"},
    {0x0903, "Scanner Controller"},
    {0x0904, "Gameport Controller"},
    {0x0980, "Other Input Device Controller"},

    // Docking Stations
    {0x0A00, "Generic Docking Station"},
    {0x0A80, "Other Docking Station"},

    // Processors
    {0x0B00, "386"},
    {0x0B01, "486"},
    {0x0B02, "Pentium"},
    {0x0B03, "Pentium Pro"},
    {0x0B10, "Alpha"},
    {0x0B20, "PowerPC"},
    {0x0B30, "MIPS"},
    {0x0B40, "Co-Processor"},
    {0x0B80, "Other Processor"},

    // Serial Bus Controllers
    {0x0C00, "FireWire (IEEE 1394) Controller"},
    {0x0C01, "ACCESS Bus Controller"},
    {0x0C02, "SSA"},
    {0x0C03, "USB Controller"},
    {0x0C04, "Fibre Channel"},
    {0x0C05, "SMBus Controller"},
    {0x0C06, "InfiniBand Controller"},
    {0x0C07, "IPMI Interface"},
    {0x0C08, "SERCOS Interface (IEC 61491)"},
    {0x0C09, "CANbus Controller"},
    {0x0C80, "Other Serial Bus Controller"},

    // Wireless Controllers
    {0x0D00, "iRDA Compatible Controller"},
    {0x0D01, "Consumer IR Controller"},
    {0x0D10, "RF Controller"},
    {0x0D11, "Bluetooth Controller"},
    {0x0D12, "Broadband Controller"},
    {0x0D20, "Ethernet Controller (802.1a)"},
    {0x0D21, "Ethernet Controller (802.1b)"},
    {0x0D80, "Other Wireless Controller"},

    // Encryption Controllers
    {0x1000, "Network and Computing Encryption/Decryption"},
    {0x1010, "Entertainment Enctyption/Decryption"},
    {0x1080, "Other Encryption Controller"},
};

char* class_finder(uint16_t class) {
    for (int i = 0; class_codes[i].classCode != 0x1080; i++) { // TODO Change this to the final option in the search list
        if (class_codes[i].classCode == class) {
            return class_codes[i].class;
        }
    }

    return "Unknown Device";
}
