#ifndef _XHCI_H
#define _XHCI_H
#include <stdint.h>

#define XHCI_MAX_DEVICES 128
#define XHCI_REG_CONFIG 0x38 // Operational Base + 0x38 - Intel XHCI Specification 1.2 - Section 5.4.7

// XHCI Capability Registers
typedef struct {
    uint8_t cap_length;     // Capability Register Length
    uint8_t rsvd;
    uint16_t hci_version;   // Interface Version Number
    uint32_t hcs_params_1;  // Structural Parameters 1
    uint32_t hcs_params_2;  // Structural Parameters 2
    uint32_t hcs_params_3;  // Structural Parameters 3
    uint32_t hcc_params_1;  // Capability Parameters 1
    uint32_t db_offset;     // Doorbell Offset
    uint32_t rts_offset;    // Runtime Register Space Offset
    uint32_t hcc_params_2;  // Capability Parameters 2
} __attribute__((packed)) XHCI_Capability_t; // Intel XHCI Specification 1.2 - Page 381

// XHCI Operational Registers
typedef struct {
    uint32_t usbcmd;        // USB Command
    uint32_t usbsts;        // USB Status
    uint32_t pagesize;      // Page Size
    uint8_t rsvd1[7];
    uint32_t dnctrl;        // Device Notification Control
    uint64_t crcr;          // Command Ring Control
    uint8_t rsvd2[15];
    // TODO Finish
} __attribute__((packed)) XHCI_Operational_t; // Intel XHCI Specification 1.2 - Page 392

// USB Command Bit Definitions (Intel XHCI Specification 1.2 - Page 393)
#define USBCMD_RS       (1 << 0)    // Run/Stop - RW
#define USBCMD_HCRST    (1 << 1)    // Host Controler Reset - RW
#define USBCMD_INTE     (1 << 2)    // Interrupter Enable - RW
#define USBCMD_HSEE     (1 << 3)    // Host System Error Enable - RW
#define USBCMD_LHCRST   (1 << 7)    // Light Host Controller Reset - RO or RW
#define USBCMD_CSS      (1 << 8)    // Controller Save State - RW
#define USBCMD_CRS      (1 << 9)    // Controller Restore State - RW
#define USBCMD_EWE      (1 << 10)   // Enable Wrap Event - RW
#define USBCMD_EU3S     (1 << 11)   // Enable U3 MFINDEX Stop - RW
#define USBCMD_CME      (1 << 13)   // CEM Enable - RW
#define USBCMD_ETE      (1 << 14)   // Extended TBC Enable - ?
#define USBCMD_TSC_EN   (1 << 15)   // Extended TBC TRB Status Enable - ?
#define USBCMD_VTIOE    (1 << 16)   // VTIO Enable - RW

// USB Status Bit Definitions (Intel XHCI Specification 1.2 - Page 398)
#define USBSTS_HCH      (1 << 0)    // HCHalted - RO
#define USBSTS_HSE      (1 << 2)    // Host System Error - RW1C
#define USBSTS_EINT     (1 << 3)    // Event Interrupt - RW1C
#define USBSTS_PCD      (1 << 4)    // Port Change Detect - RW1C
#define USBSTS_SSS      (1 << 8)    // Save State Status - RO
#define USBSTS_RSS      (1 << 9)    // Restore State Status - RO
#define USBSTS_SRE      (1 << 10)   // Save/Restore Error - RW1C
#define USBSTS_CNR      (1 << 11)   // Controller Not Ready - RO
// TODO Finish

void xhci_init(uintptr_t bar_tbl_addr);

#endif
