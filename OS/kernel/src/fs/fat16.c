#include <kernel/fs/fat16.h>
#include <kernel/drivers/ata_pio.h>
#include <kernel/memory/pmmgr.h>
#include <stdio.h>
#include <stdbool.h>

extern void* i386_kern_memset();

void fat16_fs_test() {
    printf("[FAT16] Reading LBA 0 from disk.\n");

    /*
    uintptr_t ata_pio_read
    uintptr_t ata_pio_data_ptr = (uintptr_t)pmmgr_kmalloc(1);
    i386_kern_memset((uint8_t*)ata_pio_data_ptr + 0xFFFF800000000000, 0x00, 4096);
    printf("[ATA_PIO] Sector 1, LBA 0 returned data: %llx\n", ata_pio_data_ptr + 0xFFFF800000000000);
    uint16_t* ata_pio_data = (uint16_t*)(ata_pio_data_ptr + 0xFFFF800000000000);
    for (int i = 0; i < 256; i++) {
        ata_pio_data[i] = in16(ATA_PIO_PRIMARY_BASE); // Read 2 bytes at a time
    }
    */

    uintptr_t lba0 = (uintptr_t)pmmgr_kmalloc(1); // 4KB for 512 bytes is wasteful, I know, but i havent got a slab alloc yet
    lba0 += 0xFFFF800000000000; // Add VMMGR Identity Map offset
    i386_kern_memset((uint8_t*)lba0, 0x00, 4096);
    ata_pio_read(0, lba0);
    printf("[FAT16] Raw bootsector at %llx.\n", lba0);

    fat16_parse_bootsector(lba0);
}

void fat16_parse_bootsector(uintptr_t addr) {
    // Read the BPB (BIOS Protection Block)
    BPB_t* bpb = (BPB_t*)addr;
    printf("OEM Identifier: %.8s\n", bpb->oem_identifier);
    printf("Bytes per sector: %d\n", bpb->bytes_per_sector);
    printf("Sectors per cluster: %d\n", bpb->sectors_per_cluster);
    printf("Reserved sectors: %d\n", bpb->reserved_sectors);
    printf("FAT count: %d\n", bpb->fat_count);
    printf("Total Sectors: %d\n", bpb->large_sector_count);
    printf("Sectors per fat: %d\n", bpb->sectors_per_fat);
    printf("Root dir entries: %d\n", bpb->root_dir_entries);

    // Read the EBR
    EBR_t* ebr = (EBR_t*)(addr + 0x24); // EBR is offset by 36 bytes
    printf("[EBR] System Identifier: %.8s\n", ebr->system_identifier);
    printf("[EBR] Boot Signature: %X\n", ebr->boot_signature);

    // Obtain the root directory
    int root_dir_start_sector_offset = bpb->reserved_sectors + (bpb->fat_count * bpb->sectors_per_fat); // Offset into the drive in sectors (including BPB area)
    printf("[FAT16] Root directory offset (in lba): %d\n", root_dir_start_sector_offset);

    uintptr_t root_dir_mem = addr + 0x200; // Offset 512 bytes for new heap space
    ata_pio_read(root_dir_start_sector_offset, root_dir_mem);
    // NOTE: HAVE NOT CALCULATED HOW BIG THE ROOT DIR IS, JUST WHERE IT IS. IN DEEPSEEK HISTORY

    // Calculate the number of items in the directory
    int items = 0;
    uintptr_t item_count_addr = root_dir_mem;
    while (true) {
        DIR_LFN_t* file = (DIR_LFN_t*)item_count_addr;
        if (file->order != 0x00) { // Order (first byte of the file entry) will only be 0x00 if there is no file
            items++;
            item_count_addr += 0x40; // Iterate 64 bytes to next item
        } else {
            break;
        }
    }
    printf("[FAT16] Number of items in root directory: %d\n", items);

    // List the directory
    uintptr_t file_entry_addr = root_dir_mem;
    for (int i = 0; i < items; i++) {
        DIR_LFN_t* file_entry = (DIR_LFN_t*)file_entry_addr;
        printf("File: %.8s . %.3s | File size: %d bytes\n", file_entry->info83.filename, file_entry->info83.file_extension, file_entry->info83.filesize);

        // Iterate 64 bytes to next file entry
        file_entry_addr += 0x40;
    }

    // Fetch the FAT cluster chains
}
