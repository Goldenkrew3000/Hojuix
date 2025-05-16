/*
// Hojuix PCI Driver
// 2025-05-01
// GPLv3
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <kernel.h>
#include <i386/io.h>
#include <drivers/pci.h>
#include <memory/pmmgr.h>
#include <i386/asm_functions.h>

// PCI Device Table
pci_device_table_t* pci_device_table;
int pci_device_count = 0;

uintptr_t pci_init() {
    // Allocate a page for the PCI Device Table
    uintptr_t pci_device_table_addr = pmmgr_kmalloc(1); // TODO Could potentially go over a page?
    pci_device_table_addr += 0xFFFF800000000000;
    pci_device_table = (pci_device_table_t*)pci_device_table_addr;
    //printf("[PCI] Device Table Virtual Address: 0x%llx\n", pci_device_table_addr);

    // Perform initial bus scan (Brute force method)
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
                    // Store the device in the PCI Device Table
                    pci_device_table[pci_device_count].vendor_id = vendor;
                    pci_device_table[pci_device_count].device_id = device;
                    pci_device_table[pci_device_count].class_id = class_id;
                    pci_device_table[pci_device_count].bus = i;
                    pci_device_table[pci_device_count].slot = j;
                    pci_device_table[pci_device_count].func = k;
                    pci_device_table[pci_device_count].bar_table = 0x0; // To be filled in later, but initialize it now

                    // Increment the PCI device count
                    pci_device_count++;

#if PCI_PRINT_DEVICE_TABLE
                    printf("PCI Device %04X:", vendor);
                    printf("%04X [", device);
                    printf("%04X] found. (Bus ", class_id);
                    printf("%d, Slot ", i);
                    printf("%d, Function ", j);
                    printf("%d)\n", k);
                    //printf("%s\n", pci_searchClassCode(class_id));
#endif
                }
            }
        }
    }

    printf("[PCI] Initialized.\n");
    return pci_device_table_addr;
}

// Find the AHCI controller. Returns index on success, or -ENOENT on failure
int pci_find_ahci_device() {
    for (size_t i = 0; i < pci_device_count; i++) {
        if (pci_device_table[i].class_id == PCI_CLASS_AHCI) {
            return i;
        }
    }
    return -ENOENT;
}

// Find the XHCI controller. Returns index on success, or -ENOENT on failure
int pci_find_xhci_device() {
    for (size_t i = 0; i < pci_device_count; i++) {
        if (pci_device_table[i].class_id == PCI_CLASS_XHCI) {
            return i;
        }
    }
    return -ENOENT;
}

// Find the HDA controller. Returns index on success, or -ENOENT on failure
int pci_find_hda_device() {
    for (size_t i = 0; i < pci_device_count; i++) {
        // Search for the more common 0403 class first
        if (pci_device_table[i].class_id == PCI_CLASS_HDA) {
            return i;
        }

        // Now search for the 0401 class if 0403 was not found
        if (pci_device_table[i].class_id == PCI_CLASS_HDA_B) {
            return i;
        }
    }
    return -ENOENT;
}

// Find the NVME controller. Returns index on success, or -ENOENT on failure
int pci_find_nvme_device() {
    for (size_t i = 0; i < pci_device_count; i++) {
        if (pci_device_table[i].class_id == PCI_CLASS_NVME) {
            return i;
        }
    }
    return -ENOENT;
}

// Fetch BARS of a PCI device. Index is the index of the PCI Device Table
// Returns -- on success, -EINVAL on failure TODO
uintptr_t pci_fetch_bar(int index) {
    // Allocate a page
    uintptr_t pci_bar_table_addr = pmmgr_kmalloc(1); // TODO Could also potentially go over a page?
    pci_bar_table_addr += 0xFFFF800000000000;
    i386_memset((uint8_t*)pci_bar_table_addr, 0x00, 4096); // Clear the page
    pci_device_table[index].bar_table = pci_bar_table_addr; // Fill in the PCI BAR table address
    pci_device_bar_table_t* pci_bar_table = (pci_device_bar_table_t*)pci_bar_table_addr;
    //printf("[PCI] BAR Table Virtual Address: 0x%llx\n", pci_bar_table_addr);

    // Redefine PCI Device Bus, Slot, and Function to make the code cleaner
    uint8_t pci_bus = pci_device_table[index].bus;
    uint8_t pci_slot = pci_device_table[index].slot;
    uint8_t pci_func = pci_device_table[index].func;

    // Add the PCI Device Bus, Slot, and Function to the BAR Table
    pci_bar_table->bus = pci_bus;
    pci_bar_table->slot = pci_slot;
    pci_bar_table->func = pci_func;

    // Check if PCI device has multiple functions TODO
    //uint16_t multiple_function_check = pci_readWord(pci_bus, pci_slot, pci_func, )

    // Read BAR0-5 Addresses (I could make this code recursive, but honestly there is no benefit)
    uint16_t bar0_lo = pci_readWord(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR0_LO);
    uint16_t bar0_hi = pci_readWord(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR0_HI);
    uint32_t bar0_full = (bar0_hi << 16) | bar0_lo;
    if (bar0_full != 0x0) {
        // BAR contains data
        pci_bar_table->bar0_exists = true;
        uint32_t bar0_addr = 0;
        pci_bar_table->bar0_mmio = !(bar0_full & 1);
        if (pci_bar_table->bar0_mmio) {
            bar0_addr = (bar0_full >> 4) << 4; // Clear bytes 0-3
        } else {
            bar0_addr = (bar0_full >> 2) << 2; // Clear bytes 0-1
        }
        pci_bar_table->bar0_full = bar0_full;
        pci_bar_table->bar0_addr = bar0_addr;
    } else {
        // BAR is nonexistent
        pci_bar_table->bar0_exists = false;
    }

    uint16_t bar1_lo = pci_readWord(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR1_LO);
    uint16_t bar1_hi = pci_readWord(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR1_HI);
    uint32_t bar1_full = (bar1_hi << 16) | bar1_lo;
    if (bar1_full != 0x0) {
        pci_bar_table->bar1_exists = true;
        uint32_t bar1_addr = 0;
        pci_bar_table->bar1_mmio = !(bar1_full & 1);
        if (pci_bar_table->bar1_mmio) {
            bar1_addr = (bar1_full >> 4) << 4; // Clear bytes 0-3
        } else {
            bar1_addr = (bar1_full >> 2) << 2; // Clear bytes 0-1
        }
        pci_bar_table->bar1_full = bar1_full;
        pci_bar_table->bar1_addr = bar1_addr;
    } else {
        pci_bar_table->bar1_exists = false;
    }

    uint16_t bar2_lo = pci_readWord(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR2_LO);
    uint16_t bar2_hi = pci_readWord(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR2_HI);
    uint32_t bar2_full = (bar2_hi << 16) | bar2_lo;
    if (bar2_full != 0x0) {
        pci_bar_table->bar2_exists = true;
        uint32_t bar2_addr = 0;
        pci_bar_table->bar2_mmio = !(bar2_full & 1);
        if (pci_bar_table->bar2_mmio) {
            bar2_addr = (bar2_full >> 4) << 4; // Clear bytes 0-3
        } else {
            bar2_addr = (bar2_full >> 2) << 2; // Clear bytes 0-1
        }
        pci_bar_table->bar2_full = bar2_full;
        pci_bar_table->bar2_addr = bar2_addr;
    } else {
        pci_bar_table->bar2_exists = false;
    }
    
    uint16_t bar3_lo = pci_readWord(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR3_LO);
    uint16_t bar3_hi = pci_readWord(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR3_HI);
    uint32_t bar3_full = (bar3_hi << 16) | bar3_lo;
    if (bar3_full != 0x0) {
        pci_bar_table->bar3_exists = true;
        uint32_t bar3_addr = 0;
        pci_bar_table->bar3_mmio = !(bar3_full & 1);
        if (pci_bar_table->bar3_mmio) {
            bar3_addr = (bar3_full >> 4) << 4; // Clear bytes 0-3
        } else {
            bar3_addr = (bar3_full >> 2) << 2; // Clear bytes 0-1
        }
        pci_bar_table->bar3_full = bar3_full;
        pci_bar_table->bar3_addr = bar3_addr;
    } else {
        pci_bar_table->bar3_exists = false;
    }

    uint16_t bar4_lo = pci_readWord(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR4_LO);
    uint16_t bar4_hi = pci_readWord(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR4_HI);
    uint32_t bar4_full = (bar4_hi << 16) | bar4_lo;
    if (bar4_full != 0x0) {
        pci_bar_table->bar4_exists = true;
        uint32_t bar4_addr = 0;
        pci_bar_table->bar4_mmio = !(bar4_full & 1);
        if (pci_bar_table->bar4_mmio) {
            bar4_addr = (bar4_full >> 4) << 4; // Clear bytes 0-3
        } else {
            bar4_addr = (bar4_full >> 2) << 2; // Clear bytes 0-1
        }
        pci_bar_table->bar4_full = bar4_full;
        pci_bar_table->bar4_addr = bar4_addr;
    } else {
        pci_bar_table->bar4_exists = false;
    }

    uint16_t bar5_lo = pci_readWord(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR5_LO);
    uint16_t bar5_hi = pci_readWord(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR5_HI);
    uint32_t bar5_full = (bar5_hi << 16) | bar5_lo;
    if (bar5_full != 0x0) {
        pci_bar_table->bar5_exists = true;
        uint32_t bar5_addr = 0;
        pci_bar_table->bar5_mmio = !(bar5_full & 1);
        if (pci_bar_table->bar5_mmio) {
            bar5_addr = (bar5_full >> 4) << 4; // Clear bytes 0-3
        } else {
            bar5_addr = (bar5_full >> 2) << 2; // Clear bytes 0-1
        }
        pci_bar_table->bar5_full = bar5_full;
        pci_bar_table->bar5_addr = bar5_addr;
    } else {
        pci_bar_table->bar5_exists = false;
    }

    // Disable I/O and Memory in the PCI Command register (Required to get BAR sizes)
    uint16_t pci_command_table = pci_readWord(pci_bus, pci_slot, pci_func, PCI_OFFSET_COMMAND);
    uint16_t pci_command_table_original = pci_command_table;
    pci_command_table &= ~(1 << 0); // Set Bit 0 to 0 (I/O)
    pci_command_table &= ~(1 << 1); // Set Bit 1 to 0 (Memory)
    pci_writeWord(pci_bus, pci_slot, pci_func, PCI_OFFSET_COMMAND, pci_command_table);

    // Get BAR0-5 Lengths (If BAR exists)
    if (pci_bar_table->bar0_exists) {
        uint32_t bar0_original = pci_readLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR0_LO);
        pci_writeLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR0_LO, 0xFFFFFFFF);
        uint32_t bar0_size_raw = pci_readLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR0_LO);
        pci_writeLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR0_LO, bar0_original);
        if (pci_bar_table->bar0_mmio) {
            bar0_size_raw &= 0xFFFFFFF0;
            pci_bar_table->bar0_bar_size = (~bar0_size_raw) + 1;
        } else {
            bar0_size_raw &= 0xFFFFFFFC;
            pci_bar_table->bar0_bar_size = (~bar0_size_raw) + 1;
        }
    }

    if (pci_bar_table->bar1_exists) {
        uint32_t bar1_original = pci_readLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR1_LO);
        pci_writeLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR1_LO, 0xFFFFFFFF);
        uint32_t bar1_size_raw = pci_readLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR1_LO);
        pci_writeLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR1_LO, bar1_original);
        if (pci_bar_table->bar1_mmio) {
            bar1_size_raw &= 0xFFFFFFF0;
            pci_bar_table->bar1_bar_size = (~bar1_size_raw) + 1;
        } else {
            bar1_size_raw &= 0xFFFFFFFC;
            pci_bar_table->bar1_bar_size = (~bar1_size_raw) + 1;
        }
    }

    if (pci_bar_table->bar2_exists) {
        uint32_t bar2_original = pci_readLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR2_LO);
        pci_writeLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR2_LO, 0xFFFFFFFF);
        uint32_t bar2_size_raw = pci_readLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR2_LO);
        pci_writeLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR2_LO, bar2_original);
        if (pci_bar_table->bar2_mmio) {
            bar2_size_raw &= 0xFFFFFFF0;
            pci_bar_table->bar2_bar_size = (~bar2_size_raw) + 1;
        } else {
            bar2_size_raw &= 0xFFFFFFFC;
            pci_bar_table->bar2_bar_size = (~bar2_size_raw) + 1;
        }
    }

    if (pci_bar_table->bar3_exists) {
        uint32_t bar3_original = pci_readLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR3_LO);
        pci_writeLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR3_LO, 0xFFFFFFFF);
        uint32_t bar3_size_raw = pci_readLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR3_LO);
        pci_writeLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR3_LO, bar3_original);
        if (pci_bar_table->bar3_mmio) {
            bar3_size_raw &= 0xFFFFFFF0;
            pci_bar_table->bar3_bar_size = (~bar3_size_raw) + 1;
        } else {
            bar3_size_raw &= 0xFFFFFFFC;
            pci_bar_table->bar3_bar_size = (~bar3_size_raw) + 1;
        }
    }

    if (pci_bar_table->bar4_exists) {
        uint32_t bar4_original = pci_readLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR4_LO);
        pci_writeLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR4_LO, 0xFFFFFFFF);
        uint32_t bar4_size_raw = pci_readLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR4_LO);
        pci_writeLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR4_LO, bar4_original);
        if (pci_bar_table->bar4_mmio) {
            bar4_size_raw &= 0xFFFFFFF0;
            pci_bar_table->bar4_bar_size = (~bar4_size_raw) + 1;
        } else {
            bar4_size_raw &= 0xFFFFFFFC;
            pci_bar_table->bar4_bar_size = (~bar4_size_raw) + 1;
        }
    }

    if (pci_bar_table->bar5_exists) {
        uint32_t bar5_original = pci_readLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR5_LO);
        pci_writeLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR5_LO, 0xFFFFFFFF);
        uint32_t bar5_size_raw = pci_readLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR5_LO);
        pci_writeLong(pci_bus, pci_slot, pci_func, PCI_OFFSET_BAR5_LO, bar5_original);
        if (pci_bar_table->bar5_mmio) {
            bar5_size_raw &= 0xFFFFFFF0;
            pci_bar_table->bar5_bar_size = (~bar5_size_raw) + 1;
        } else {
            bar5_size_raw &= 0xFFFFFFFC;
            pci_bar_table->bar5_bar_size = (~bar5_size_raw) + 1;
        }
    }

    // Write original PCI Command register back
    pci_writeWord(pci_bus, pci_slot, pci_func, PCI_OFFSET_COMMAND, pci_command_table_original);

    return pci_bar_table_addr;
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

void pci_writeWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t value) {
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    // Create configuration address
    address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    // Write out the address
    out32(PCI_CONFIG_ADDR, address);

    // Write the data
    out16(PCI_CONFIG_DATA, value);
}

uint32_t pci_readLong(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp = 0;

    // Create configuration address
    address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    // Write out the address
    out32(PCI_CONFIG_ADDR, address);

    // Read in the data
    tmp = in32(PCI_CONFIG_DATA);
    return tmp;
}

void pci_writeLong(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    // Create configuration address
    address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    // Write out the address
    out32(PCI_CONFIG_ADDR, address);

    // Write the data
    out32(PCI_CONFIG_DATA, value);
}

char* pci_searchClassCode(uint16_t class) {
    for (int i = 0; pci_classCodes[i].classCode != 0xFFFF; i++) {
        if (pci_classCodes[i].classCode == class) {
            return pci_classCodes[i].className;
        }
    }

    return "Unknown";
}

/*
KEEP
// Forge NVME address
    uint64_t nvme_base_addr = (uint64_t)(((uint64_t)bar1_full << 32) | (bar0_full & 0xFFFFFFF0));
    printf("NVME Base Address: 0x%llx\n", nvme_base_addr);
    uint64_t nvme_cap_strd = (nvme_base_addr >> 12) & 0xF;
    printf("NVME CAP STRD: 0x%llx\n", nvme_cap_strd);

    uint16_t b10 = pci_readWord(bus, slot, func, 0x10);
    if (b10 & (1 << 7)) {
        printf("Device has multiple functions\n");
    } else { printf("Device does not have multiple functions\n");}
*/
