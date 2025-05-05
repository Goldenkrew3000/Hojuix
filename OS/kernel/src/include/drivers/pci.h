#ifndef _PCI_H
#define _PCI_H
#include <stdint.h>
#include <stdbool.h>

// PCI Addresses
#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

// PCI Offsets
#define PCI_OFFSET_COMMAND 0x4
#define PCI_OFFSET_STATUS  0x6
#define PCI_OFFSET_BAR0_LO 0x10
#define PCI_OFFSET_BAR0_HI 0x12
#define PCI_OFFSET_BAR1_LO 0x14
#define PCI_OFFSET_BAR1_HI 0x16
#define PCI_OFFSET_BAR2_LO 0x18
#define PCI_OFFSET_BAR2_HI 0x1A
#define PCI_OFFSET_BAR3_LO 0x1C
#define PCI_OFFSET_BAR3_HI 0x1E
#define PCI_OFFSET_BAR4_LO 0x20
#define PCI_OFFSET_BAR4_HI 0x22
#define PCI_OFFSET_BAR5_LO 0x24
#define PCI_OFFSET_BAR5_HI 0x26

// PCI Classes
#define PCI_CLASS_AHCI 0x0106
#define PCI_CLASS_XHCI 0x0C03

typedef struct {
    uint16_t vendor_id; // Vendor ID
    uint16_t device_id; // Device ID
    uint16_t class_id;  // Class ID
    uint8_t bus;        // Location on PCI Bus (0 - 255)
    uint8_t slot;       // Location on bus (0 - 31)
    uint8_t func;       // Location on slot (0 - 7)
    uintptr_t bar_table;
} pci_device_table_t;

typedef struct {
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    
    bool bar0_exists;
    uint32_t bar0_addr;
    uint32_t bar0_full; // Sometimes the full uint32_t is required for fetching 64-bit addresses, safer to keep it
    uint32_t bar0_bar_size;
    bool bar0_mmio;

    bool bar1_exists;
    uint32_t bar1_addr;
    uint32_t bar1_full;
    uint32_t bar1_bar_size;
    bool bar1_mmio;

    bool bar2_exists;
    uint32_t bar2_addr;
    uint32_t bar2_full;
    uint32_t bar2_bar_size;
    bool bar2_mmio;
    
    bool bar3_exists;
    uint32_t bar3_addr;
    uint32_t bar3_full;
    uint32_t bar3_bar_size;
    bool bar3_mmio;
    
    bool bar4_exists;
    uint32_t bar4_addr;
    uint32_t bar4_full;
    uint32_t bar4_bar_size;
    bool bar4_mmio;

    bool bar5_exists;
    uint32_t bar5_addr;
    uint32_t bar5_full;
    uint32_t bar5_bar_size;
    bool bar5_mmio;
} pci_device_bar_table_t;

// PCI Device Type용
typedef struct {
    uint16_t classCode;
    char* className;
} t_pci_class;

// PCI Device Type Full Name용
typedef struct {
    uint16_t vendor;
    uint16_t device;
    char* deviceName;
} t_pci_device_name;

static t_pci_class pci_classCodes[] = {
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

uintptr_t pci_init();
int pci_find_ahci_device();
int pci_find_xhci_device();
uintptr_t pci_fetch_bar(int index);
uint16_t pci_readWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_writeWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t value);
uint32_t pci_readLong(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_writeLong(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
char* pci_searchClassCode(uint16_t class);
char* pci_searchDevices(uint16_t vendor, uint16_t device);

#endif
