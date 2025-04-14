#include <kernel/drivers/ata_pio.h>
#include <kernel/i386/io.h>
#include <kernel/memory/pmmgr.h>
#include <stdio.h>

extern uint8_t* i386_kern_memset();

void ata_pio_init() {
    printf("[ATA_PIO] Running Sequence...\n");

    // IDENTIFY the device
    // To select the target drive, send 0xA0 for master, or 0xB0 for slave to the drive select IO port
    out8(ATA_PIO_PRIMARY_BASE + ATA_PIO_REG_SELECT_DRV, 0xA0);

    // Now set Sector count, LBAlo, LBAmid, and LBAhi to 0
    out8(ATA_PIO_PRIMARY_BASE + ATA_PIO_REG_SECTOR_COUNT, 0x00);
    out8(ATA_PIO_PRIMARY_BASE + ATA_PIO_REG_LBA_LO, 0x00);
    out8(ATA_PIO_PRIMARY_BASE + ATA_PIO_REG_LBA_MID, 0x00);
    out8(ATA_PIO_PRIMARY_BASE + ATA_PIO_REG_LBA_HI, 0x00);

    // Now send the IDENTIFY command (0xEC) to the Command IO port
    out8(ATA_PIO_PRIMARY_BASE + ATA_PIO_REG_STAT_CMD, ATA_PIO_CMD_IDENTIFY);

    // Now read the status (also command) port
    uint8_t identify_status = in8(ATA_PIO_PRIMARY_BASE + ATA_PIO_REG_STAT_CMD);
    printf("[ATA_PIO] Initial ATA Drive Status: %02X\n", identify_status);
    if (identify_status != 0x00) {
        printf("Waiting for BSY to clear.\n");
        while (in8(ATA_PIO_PRIMARY_BASE + ATA_PIO_REG_STAT_CMD) & 0x80 != 0) { // Bitmask Bit 7 (Left most)
            // Spin here until bit 7 (BSY/Busy is set to 0)
        }
    } else {
        printf("Drive does not exist (0x00).\n");
    }

    // Now (as long as ERR (bit 0) is not set TODO), read 256 16bit values from the DATA (Base) port
    uintptr_t ata_pio_identify_data_ptr = (uintptr_t)pmmgr_kmalloc(1);
    i386_kern_memset((uint8_t*)ata_pio_identify_data_ptr + 0xFFFF800000000000, 0x00, 4096);
    printf("[ATA_PIO] IDENTIFY returned data: %llx\n", ata_pio_identify_data_ptr + 0xFFFF800000000000);
    uint16_t* ata_pio_identify_data = (uint16_t*)(ata_pio_identify_data_ptr + 0xFFFF800000000000);
    for (int i = 0; i < 256; i++) {
        ata_pio_identify_data[i] = in16(ATA_PIO_PRIMARY_BASE);
    }

    // Read Sector 1, LBA 0 from the disk (Boot sector)

    // Send sector count (1 sector = 512 bytes)
    // // Select LBA 0 (Master Drive)
    //out8(0x1F6, 0xE0); // 0xE0 = Master, LBA mode

    /*
    out8(ATA_PIO_PRIMARY_BASE + ATA_PIO_REG_SELECT_DRV, 0xE0);

    // Send sector count (1 sector)
    out8(ATA_PIO_PRIMARY_BASE + ATA_PIO_REG_SECTOR_COUNT, 1);

    // Send LBA address (LBA 0)
    out8(ATA_PIO_PRIMARY_BASE + ATA_PIO_REG_LBA_LO, 0);
    out8(ATA_PIO_PRIMARY_BASE + ATA_PIO_REG_LBA_MID, 0);
    out8(ATA_PIO_PRIMARY_BASE + ATA_PIO_REG_LBA_HI, 0);

    // Send READ command (0x20)
    out8(ATA_PIO_PRIMARY_BASE + ATA_PIO_REG_STAT_CMD, ATA_PIO_CMD_READ);

    //hmm
    while (!(in8(0x1F7) & 0x08));

    uintptr_t ata_pio_data_ptr = (uintptr_t)pmmgr_kmalloc(1);
    i386_kern_memset((uint8_t*)ata_pio_data_ptr + 0xFFFF800000000000, 0x00, 4096);
    printf("[ATA_PIO] Sector 1, LBA 0 returned data: %llx\n", ata_pio_data_ptr + 0xFFFF800000000000);
    uint16_t* ata_pio_data = (uint16_t*)(ata_pio_data_ptr + 0xFFFF800000000000);
    for (int i = 0; i < 256; i++) {
        ata_pio_data[i] = in16(ATA_PIO_PRIMARY_BASE); // Read 2 bytes at a time
    }
    */
}

void ata_pio_read(int lba, uintptr_t addr) { // Send zero'd virtual address
    out8(ATA_PIO_PRIMARY_BASE + ATA_PIO_REG_SELECT_DRV, 0xE0);

    // Send sector count (1 sector)
    out8(ATA_PIO_PRIMARY_BASE + ATA_PIO_REG_SECTOR_COUNT, 1);

    // Send LBA address (LBA 0)
    out8(ATA_PIO_PRIMARY_BASE + ATA_PIO_REG_LBA_LO, (uint8_t)(lba & 0xFF));
    out8(ATA_PIO_PRIMARY_BASE + ATA_PIO_REG_LBA_MID, (uint8_t)((lba >> 8) & 0xFF));
    out8(ATA_PIO_PRIMARY_BASE + ATA_PIO_REG_LBA_HI, (uint8_t)((lba >> 16) & 0xFF));

    // Send READ command (0x20)
    out8(ATA_PIO_PRIMARY_BASE + ATA_PIO_REG_STAT_CMD, ATA_PIO_CMD_READ);

    while (!(in8(0x1F7) & 0x08));

    printf("[ATA_PIO] Read LBA %d (%llx).\n", lba, addr);
    uint16_t* ata_pio_data = (uint16_t*)addr;
    for (int i = 0; i < 256; i++) {
        ata_pio_data[i] = in16(ATA_PIO_PRIMARY_BASE);
    }
}
