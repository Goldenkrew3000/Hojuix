#include <kern/kprintf.h>
#include <drivers/xhci.h>
#include <drivers/pci.h>
#include <memory/vmmgr.h>

void xhci_mmio_write(uint64_t reg, uint32_t val);

void xhci_init(uintptr_t bar_tbl_addr) {
    printf("[XHCI] Init.\n");

    // Load the PCI BAR Table
    pci_device_bar_table_t* pci_bar_tbl = (pci_device_bar_table_t*)bar_tbl_addr;

    // Enable interrupts, DMA, and memory space access in the PCI Command Register (Bit 1, 2, 10 is off)
    // Not sure what I need for XHCI, but I would guess this and it doesnt hurt so why not
    uint16_t pci_command_register = pci_readWord(pci_bar_tbl->bus, pci_bar_tbl->slot, pci_bar_tbl->func, PCI_OFFSET_COMMAND);
    pci_command_register &= ~(0x1 << 9);
    pci_command_register |= (0x3 << 1);
    pci_writeWord(pci_bar_tbl->bus, pci_bar_tbl->slot, pci_bar_tbl->func, PCI_OFFSET_COMMAND, pci_command_register);

    // Forge 64-bit base address from BAR0 and BAR1 (Intel XHCI Specification 1.2 - Page 594)
    // This address points to the 'xHCI PF0 MMIO Space, or referred to as PBAR0'
    uint64_t xhci_base_addr = (uint64_t)(((uint64_t)pci_bar_tbl->bar1_full << 32) | (pci_bar_tbl->bar0_full & 0xFFFFFFF0));
    printf("XHCI Base Address: 0x%llx\n", xhci_base_addr);

    // Map the XHCI base address
    vmmgr_mmio_map_uncache(xhci_base_addr, xhci_base_addr, 1);

    // Load the capabilities and operational registers
    XHCI_Capability_t* xhci_capability = (XHCI_Capability_t*)xhci_base_addr;
    XHCI_Operational_t* xhci_operational = (XHCI_Operational_t*)(xhci_base_addr + xhci_capability->cap_length);
    printf("XHCI Version: 0x%lx\n", xhci_capability->hci_version);

    // Reset the controller
    printf("USBSTS Halted: %d\n", xhci_operational->usbsts & USBSTS_HCH); // Print bit 0 of USBSTS / HCH
    if (xhci_operational->usbsts & USBSTS_HCH != 1) {
        printf("[XHCI] Critical issue - xHCI controller is not halted, unknown state detected.\n");
        return; // TODO RETURN FAILURE
    }
    // Set USBCMD Bit 1 (HCRST) to 1, and when the bit is set to 0, the reset is completed
    xhci_operational->usbcmd |= USBCMD_HCRST;
    while (xhci_operational->usbcmd & USBCMD_HCRST != 0) { } // TODO Add spin
    printf("[XHCI] Reset complete.\n");

    // Wait for CNR (Bit 11) flag in USBSTS to be 0
    while (xhci_operational->usbcmd & USBSTS_CNR != 0) { } // TODO Add spin
    
    // Check that the xHCI controller supports a 4096 byte page size
    if (xhci_operational->pagesize & (1 << 0) == 0) {
        printf("[XHCI] Critical issue - xHCI controller does not support 4096 byte page size.\n");
        return; // TODO RETURN FAILURE
    }

    // Read the port count from the compatibility register (HCSPARAMS1 Bit 24 - 31)
    uint8_t xhci_port_count = (((xhci_capability->hcs_params_1) >> 24) & 0xff);
    if (xhci_port_count == 0) {
        printf("[XHCI] Invalid port count returned from xHCI controller.\n");
        return; // TODO RETURN FAILURE
    }
    printf("[XHCI] Port count: %d\n", xhci_port_count);

    // Read the slot count (HCSPARAMS1 Bit 0 - 7), and enable the slots in the CONFIG register
    uint8_t xhci_slot_count = (((xhci_capability->hcs_params_1) >> 0) & 0xff);
    if (xhci_slot_count > XHCI_MAX_DEVICES) {
        xhci_slot_count = XHCI_MAX_DEVICES;
    }
    printf("[XHCI] Slot count: %d\n", xhci_slot_count);
    uintptr_t xhci_reg_config_base = xhci_base_addr + xhci_capability->cap_length + XHCI_REG_CONFIG; // Operational Base + XHCI_REG_CONFIG
    xhci_mmio_write((uint64_t)xhci_reg_config_base, xhci_slot_count);

    // (Haiku does this) Find out what protocol is used for each port

    // Allocate the DCBAAP (Device Context Base Address Array Pointer)

    // Define the Command Ring Dequeue Pointer

    // Start the XHCI Controller (Set bit 0 of USBCMD to 1), and wait for USBSTS_HCH to be 0
    printf("[XHCI] Controller is configured, starting controller.\n");
    xhci_operational->usbcmd |= USBCMD_RS;
    while (xhci_operational->usbsts & USBSTS_HCH != 0) { } // TODO Add Spin




    printf("[XHCI] Controller is ready.\n");
}

void xhci_mmio_write(uint64_t reg, uint32_t val) {
	*(volatile uint64_t *)(reg) = val;
}
