#ifndef _ATA_PIO_H
#define _ATA_PIO_H
#include <stdint.h>

/*
// figuring out what the fuck TODO
//
// Primary IDE bus: 0x1F0 - 0x1F7
// Seconary IDE bus: 0x170 - 0x177
// Primary Control Register: 0x3F6
// Secondary Control Register: 0x376
// Primary IRQ: 14
// Secondary IRQ: 15
*/

#define ATA_PIO_PRIMARY_BASE 0x1F0      // Primary IDE Base Register
#define ATA_PIO_REG_FEAT_ERR 0x1        // IDE Error Register Offset
#define ATA_PIO_REG_SECTOR_COUNT 0x2    // ---
#define ATA_PIO_REG_LBA_LO 0x3          // ---
#define ATA_PIO_REG_LBA_MID 0x4         // ---
#define ATA_PIO_REG_LBA_HI 0x5          // ---
#define ATA_PIO_REG_SELECT_DRV 0x6      // ---
#define ATA_PIO_REG_STAT_CMD 0x7        // ---
#define ATA_PIO_PRIMARY_CTRL 0x3F6      // Primary IDE Control Register

#define ATA_PIO_CMD_IDENTIFY 0xEC
#define ATA_PIO_CMD_READ 0x20

void ata_pio_init();
void ata_pio_read(int lba, uintptr_t addr);

#endif
