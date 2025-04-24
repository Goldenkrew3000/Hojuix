#ifndef _PCI_H
#define _PCI_H

// PCI Device Table용
struct t_pci_device {
    uint16_t vendor_id; // Vendor ID
    uint16_t device_id; // Device ID
    uint16_t class_id; // Class ID
    uint8_t bus; // Location on PCI Bus (0 - 255)
    uint8_t slot; // Location on bus (0 - 31)
    uint8_t function; // Location on slot (0 - 7)
} __attribute__((packed));

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

static t_pci_device_name pci_deviceNames[] = {
    // Intel 0x8086
    {0x8086, 0x0007, "82379AB"},
    {0x8086, 0x0008, "Extended Express System Support Controller"},
    {0x8086, 0x0039, "21145 Fast Ethernet"},
    {0x8086, 0x0040, "Core Processor DRAM Controller"},
    {0x8086, 0x0041, "Core Processor PCI Express x16 Root Port"},
    {0x8086, 0x0042, "Core Processor Integrated Graphics Controller"},
    {0x8086, 0x0043, "Core Processor Secondary PCI Express Root Port"},
    {0x8086, 0x0044, "Core Processor DRAM Controller"},
    {0x8086, 0x0045, "Core Processor PCI Express x16 Root Port"},
    {0x8086, 0x0046, "Core Processor Integrated Graphics Controller"},
    {0x8086, 0x0047, "Core Processor Secondary PCI Express Root Port"},
    {0x8086, 0x0048, "Core Processor DRAM Controller"},
    {0x8086, 0x0049, "Core Processor PCI Express x16 Root Port"},
    {0x8086, 0x004A, "Core Processor Integrated Graphics Controller"},
    {0x8086, 0x004B, "Core Processor Secondary PCI Express Root Port"},
    {0x8086, 0x0050, "Core Processor Thermal Management Controller"},
    {0x8086, 0x0069, "Core Processor DRAM Controller"},
    {0x8086, 0x0082, "Centrino Advanced-N 6205 [Taylor Peak]"},
    {0x8086, 0x0083, "Centrino Wireless-N 1000 [Condor Peak]"},
    {0x8086, 0x0084, "Centrino Wireless-N 1000 [Condor Peak]"},
    {0x8086, 0x0085, "Centrino Advanced-N 6205 [Taylor Peak]"},
    {0x8086, 0x0087, "Centrino Advanced-N + WiMAX 6250 [Kilmer Peak]"},
    {0x8086, 0x0089, "Centrino Advanced-N + WiMAX 6250 [Kilmer Peak]"},
    {0x8086, 0x008A, "Centrino Wireless-N 1030 [Rainbow Peak]"},
    {0x8086, 0x008B, "Centrino Wireless-N 1030 [Rainbow Peak]"},
    {0x8086, 0x0090, "Centrino Advanced-N 6230 [Rainbow Peak]"},
    {0x8086, 0x0091, "Centrino Advanced-N 6230 [Rainbow Peak]"},
    {0x8086, 0x0100, "2nd Generation Core Processor Family DRAM Controller"},
    {0x8086, 0x0101, "Xeon E3-1200/2nd Generation Core Processor Family PCI Express Root Port"},
    {0x8086, 0x0102, "2nd Generation Core Processor Family Integrated Graphics Controller"},
    {0x8086, 0x0104, "2nd Generation Core Processor Family DRAM Controller"},
    {0x8086, 0x0105, "Xeon E3-1200/2nd Generation Core Processor Family PCI Express Root Port"},
    {0x8086, 0x0106, "2nd Generation Core Processor Family Integrated Graphics Controller"},
    {0x8086, 0x0108, "Xeon E3-1200 Processor Family DRAM Controller"},
    {0x8086, 0x0109, "Xeon E3-1200/2nd Generation Core Processor Family PCI Express Root Port"},
    {0x8086, 0x010A, "Xeon E3-1200 Processor Family Integrated Graphics Controller"},
    {0x8086, 0x010B, "Xeon E3-1200/2nd Generation Core Processor Family Integrated Graphics Controller"},
    {0x8086, 0x010C, "Xeon E3-1200/2nd Generation Core Processor Family DRAM Controller"},
    {0x8086, 0x010D, "Xeon E3-1200/2nd Generation Core Processor Family PCI Express Root Port"},
    {0x8086, 0x010E, "Xeon E3-1200/2nd Generation Core Processor Family Integrated Graphics Controller"},
    {0x8086, 0x0112, "2nd Generation Core Processor Family Integrated Graphics Controller"},
    {0x8086, 0x0116, "2nd Generation Core Processor Family Integrated Graphics Controller"},
    {0x8086, 0x0122, "2nd Generation Core Processor Family Integrated Graphics Controller"},
    {0x8086, 0x0126, "2nd Generation Core Processor Family Integrated Graphics Controller"},
    {0x8086, 0x0150, "Xeon E3-1200 v2/3rd Gen Core processor DRAM Controller"},
    {0x8086, 0x0151, "Xeon E3-1200 v2/3rd Gen Core processor PCI Express Root Port"},
    {0x8086, 0x0152, "Xeon E3-1200 v2/3rd Gen Core processor Graphics Controller"},
    {0x8086, 0x0153, "3rd Gen Core Processor Thermal Subsystem"},
    {0x8086, 0x0154, "3rd Gen Core processor DRAM Controller"},
    {0x8086, 0x0155, "Xeon E3-1200 v2/3rd Gen Core processor PCI Express Root Port"},
    {0x8086, 0x0156, "3rd Gen Core processor Graphics Controller"},
    {0x8086, 0x0158, "Xeon E3-1200 v2/Ivy Bridge DRAM Controller"},
    {0x8086, 0x0159, "Xeon E3-1200 v2/3rd Gen Core processor PCI Express Root Port"},
    {0x8086, 0x015A, "Xeon E3-1200 v2/Ivy Bridge Graphics Controller"},
    {0x8086, 0x015C, "Xeon E3-1200 v2/3rd Gen Core processor DRAM Controller"},
    {0x8086, 0x015D, "Xeon E3-1200 v2/3rd Gen Core processor PCI Express Root Port"},
    {0x8086, 0x015E, "Xeon E3-1200 v2/3rd Gen Core processor Graphics Controller"},
    {0x8086, 0x0162, "IvyBridge GT2 [HD Graphics 4000]"},
    {0x8086, 0x0166, "3rd Gen Core processor Graphics Controller"},
    {0x8086, 0x016A, "Xeon E3-1200 v2/3rd Gen Core processor Graphics Controller"},
    {0x8086, 0x0172, "Xeon E3-1200 v2/3rd Gen Core processor Graphics Controller"},
    {0x8086, 0x0176, "3rd Gen Core processor Graphics Controller"},
    {0x8086, 0x0201, "Arctic Sound"},
    {0x8086, 0x2918, "82801IB (ICH9) LPC Interface Controller"},
};

void pci_init();
uint16_t pci_readWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
char* pci_searchClassCode(uint16_t class);
char* pci_searchDevices(uint16_t vendor, uint16_t device);

#endif
