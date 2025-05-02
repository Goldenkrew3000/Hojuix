#ifndef _AHCI_H
#define _AHCI_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define AHCI_MAX_PORTS 32 // Max amount of allowed ports on a SATA bus
#define AHCI_MAX_CMDS 32
#define AHCI_PRDT_SIZE 8

typedef volatile struct {
    uint32_t clb;		// 0x00,  list base address, 1K-byte aligned
    uint32_t clbu;		// 0x04, command list base address upper 32 bits
    uint32_t fb;		// 0x08, FIS base address, 256-byte aligned
    uint32_t fbu;		// 0x0C, FIS base address upper 32 bits
    uint32_t is;		// 0x10, interrupt status
    uint32_t ie;		// 0x14, interrupt enable
    uint32_t cmd;		// 0x18, command and status
    uint32_t rsv0;		// 0x1C, Reserved
    uint32_t tfd;		// 0x20, task file data
    uint32_t sig;		// 0x24, signature
    uint32_t ssts;		// 0x28, SATA status (SCR0:SStatus)
    uint32_t sctl;		// 0x2C, SATA control (SCR2:SControl)
    uint32_t serr;		// 0x30, SATA error (SCR1:SError)
    uint32_t sact;		// 0x34, SATA active (SCR3:SActive)
    uint32_t ci;		// 0x38, command issue
    uint32_t sntf;		// 0x3C, SATA notification (SCR4:SNotification)
    uint32_t fbs;		// 0x40, FIS-based switch control
    uint32_t rsv1[11];	// 0x44 ~ 0x6F, Reserved
    uint32_t vendor[4];	// 0x70 ~ 0x7F, vendor specific
} hba_port_t; // 128 bytes

typedef volatile struct
{
    // 0x00 - 0x2B, Generic Host Control
    uint32_t cap;		// 0x00, Host capability
    uint32_t ghc;		// 0x04, Global host control
    uint32_t is;		// 0x08, Interrupt status
    uint32_t pi;		// 0x0C, Port implemented
    uint32_t vs;		// 0x10, Version
    uint32_t ccc_ctl;	// 0x14, Command completion coalescing control
    uint32_t ccc_pts;	// 0x18, Command completion coalescing ports
    uint32_t em_loc;		// 0x1C, Enclosure management location
    uint32_t em_ctl;		// 0x20, Enclosure management control
    uint32_t cap2;		// 0x24, Host capabilities extended
    uint32_t bohc;		// 0x28, BIOS/OS handoff control and status

    uint32_t  rsv[29];

    // 0xA0 - 0xFF, Vendor specific registers
    uint32_t  vendor[24];

    // 0x100 - 0x10FF, Port control registers
    hba_port_t	ports[32];	// 1 ~ 32
} hba_mem_t; // 256 bytes without the ports

typedef struct
{
    // DW0
    uint8_t  cfl:5;		// Command FIS length in DWORDS, 2 ~ 16
    uint8_t  a:1;		// ATAPI
    uint8_t  w:1;		// Write, 1: H2D, 0: D2H
    uint8_t  p:1;		// Prefetchable

    uint8_t  r:1;		// Reset
    uint8_t  b:1;		// BIST
    uint8_t  c:1;		// Clear busy upon R_OK
    uint8_t  rsv0:1;		// Reserved
    uint8_t  pmp:4;		// Port multiplier port

    uint16_t prdtl;		// Physical region descriptor table length in entries

    // DW1
    volatile
    uint32_t prdbc;		// Physical region descriptor byte count transferred

    // DW2, 3
    uint32_t ctba;		// Command table descriptor base address
    uint32_t ctbau;		// Command table descriptor base address upper 32 bits

    // DW4 - 7
    uint32_t rsv1[4];	// Reserved
} hba_command_hdr_t;

typedef struct
{
    // DWORD 0
    uint8_t  fis_type;	// FIS_TYPE_REG_H2D

    uint8_t  pmport:4;	// Port multiplier
    uint8_t  rsv0:3;		// Reserved
    uint8_t  c:1;		// 1: Command, 0: Control

    uint8_t  command;	// Command register
    uint8_t  featurel;	// Feature register, 7:0
    
    // DWORD 1
    uint8_t  lba0;		// LBA low register, 7:0
    uint8_t  lba1;		// LBA mid register, 15:8
    uint8_t  lba2;		// LBA high register, 23:16
    uint8_t  device;		// Device register

    // DWORD 2
    uint8_t  lba3;		// LBA register, 31:24
    uint8_t  lba4;		// LBA register, 39:32
    uint8_t  lba5;		// LBA register, 47:40
    uint8_t  featureh;	// Feature register, 15:8

    // DWORD 3
    uint8_t  countl;		// Count register, 7:0
    uint8_t  counth;		// Count register, 15:8
    uint8_t  icc;		// Isochronous command completion
    uint8_t  control;	// Control register

    // DWORD 4
    uint8_t  rsv1[4];	// Reserved
} FIS_REG_H2D_t;




typedef struct
{
    uint32_t dba;       // Data base address
    uint32_t dbau;      // Data base address (upper 32 bits)
    uint32_t rsvd0;     // Reserved
    uint32_t dbc :22;   // Byte count, 4M max
    uint32_t rsvd1 :9;  // Reserved
    uint32_t i :1;      // Interrupt on completion
} hba_prdt_entry_t;

typedef struct
{
    uint8_t cfis[64];               // (Offset 0x0)  - Command FIS
    uint8_t acmd[16];               // (Offset 0x40) - ATAPI command, 12 or 16 bytes
    uint8_t rsvd[48];               // (Offset 0x50) - Reserved
    hba_prdt_entry_t prdt_entry[1]; // (Offset 0x80) - Physical region descriptor table entries, 0 ~ 65535
} hba_command_table_t;








typedef volatile struct { // (AHCI 1.3.1 Page 23)
    uint8_t np :5;      // Number of ports                              (Bits 0 - 4)
    uint8_t sxs :1;     // Supports external sata (eSATA)               (Bit 5)
    uint8_t ems :1;     // Enclosure Management Supported               (Bit 6)
    uint8_t cccs :1;    // Command Completion Coalescing Supported      (Bit 7)
    uint8_t ncs :5;     // Number of command slots                      (Bits 8 - 12)
    uint8_t psc :1;     // Partial State Capable                        (Bit 13)
    uint8_t ssc :1;     // Slumber State Capable                        (Bit 14)
    uint8_t pmd :1;     // PIO Multiple DRQ Block                       (Bit 15)
    uint8_t fbss :1;    // FIS-Based Switching Supported                (Bit 16)
    uint8_t spm :1;     // Supports Port Multiplier                     (Bit 17)
    uint8_t sam :1;     // Supports ACHI mode only                      (Bit 18)
    uint8_t rsvd1 :1;   // Reserved                                     (Bit 19)
    uint8_t iis :4;     // Interface Speed Support                      (Bits 20 - 23)
    uint8_t sclo :1;    // Supports Command List Override               (Bit 24)
    uint8_t sal :1;     // Supports Activity LED                        (Bit 25)
    uint8_t salp :1;    // Supports Aggressive Link Power Management    (Bit 26)
    uint8_t sss :1;     // Supports Staged Spin-up                      (Bit 27)
    uint8_t smps :1;    // Supports Mechanical Presence Switch          (Bit 28)
    uint8_t ssntf :1;   // Supports SNotification Register              (Bit 29)
    uint8_t sncq :1;    // Supports Native Command Queuing              (Bit 30)
    uint8_t s64a :1;    // Supports 64-bit Addressing                   (Bit 31)
} hba_capabilities_t;

int ahci_init(uintptr_t bar_tbl_addr);
int ahci_read(int port, uintptr_t buffer, uint64_t lba_start, uint32_t sector_count);
void ahci_interrupt(void* frame);

#endif
