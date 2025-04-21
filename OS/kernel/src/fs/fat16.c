/*
// FAT16 Kernel Driver v1.0
// Hojuix 2025-04-19
// POTENTIAL ISSUE: I am only reading in 32 sectors, even though the root directory size is 32.9 sectors
//                  This would only be an issue if you are REALLY pushing the filesystem to the limit (256 files in the root directory)
// ISSUE: Issue random memory addresses through the VMMGR after I refactor the memory subsystem
*/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <kernel/fs/fat16.h>
#include <kernel/drivers/ata_pio.h>
#include <kernel/memory/pmmgr.h>
#include <kernel/memory/vmmgr.h>

extern void* i386_kern_memset(); // Fix this eventually
#define DIV_ROUND_UP(a, b) (((a) + (b) - 1) / (b))

BPB_t* bpb = NULL;
EBR_t* ebr = NULL;
int root_dir_start_sector_offset = 0;
int root_dir_size_sectors = 0;
int data_start_sector = 0;
int root_dir_items = 0;
fat16_file_t files[32]; // Support 32 files

void fat16_fs_test() {
    printf("[FAT16] Reading LBA 0 from disk.\n");

    uintptr_t lba0 = (uintptr_t)pmmgr_kmalloc(1); // 4KB for 512 bytes is wasteful, I know, but i havent got a slab alloc yet
    lba0 += 0xFFFF800000000000; // Add VMMGR Identity Map offset
    i386_kern_memset((uint8_t*)lba0, 0x00, 4096);
    ata_pio_read(0, lba0);
    printf("[FAT16] Raw bootsector at %llx.\n", lba0);

    fat16_parse_bootsector(lba0);
    fat16_read_root_directory();
    fat16_read_file(files[2].start_cluster, files[2].filesize);
}

void fat16_parse_bootsector(uintptr_t addr) {
    // Read the BPB (BIOS Protection Block)
    bpb = (BPB_t*)addr;
    printf("[BPB] Made with '%.8s'\n", bpb->oem_identifier);

    // Read the EBR (Extended Boot Record)
    ebr = (EBR_t*)(addr + 0x24); // EBR is offset by 36 bytes
    printf("[EBR] Filesystem is '%.5s', Bootable: ", ebr->system_identifier);
    if (ebr->boot_signature == 0xAA55) {
        printf("Yes\n");
    } else {
        printf("No\n");
    }
}

void fat16_read_root_directory() {
    // Obtain the position of the root directory
    root_dir_start_sector_offset = bpb->reserved_sectors + (bpb->fat_count * bpb->sectors_per_fat); // Offset into the drive in sectors (including BPB area)
    root_dir_size_sectors = (bpb->root_dir_entries * 32 + bpb->bytes_per_sector - 1) / bpb->bytes_per_sector; // This is always like 32.9 sectors, rounds down?

    // Calculate where the data actually starts in the FAT
    data_start_sector = root_dir_start_sector_offset + root_dir_size_sectors;

    // Allocate memory for the root directory
    uintptr_t root_dir_virtual_addr = 0x9000000000;
    vmmgr_kalloc_page(root_dir_virtual_addr, root_dir_size_sectors / (4096 / bpb->bytes_per_sector));
    i386_kern_memset((uint8_t*)root_dir_virtual_addr, 0x00, root_dir_size_sectors * bpb->bytes_per_sector);

    // Read in the root directory
    for (size_t i = 0; i < root_dir_size_sectors; i++) {
        ata_pio_read(root_dir_start_sector_offset + i, root_dir_virtual_addr + (0x200 * i));
    }

    // Count the number of items in the root directory
    uintptr_t root_dir_item_addr = root_dir_virtual_addr;
    while (true) {
        DIR_LFN_t* root_dir_item = (DIR_LFN_t*)root_dir_item_addr;
        if (root_dir_item->order != 0x00) { // Order (first byte of the file entry) will only be 0x00 if there is no file
            root_dir_items++;
            root_dir_item_addr += 0x40; // Iterate 64 bytes to next item
        } else {
            break;
        }
    }
    printf("[FAT16] Number of items in root directory: %d\n", root_dir_items);

    // List the items in the root directory
    root_dir_item_addr = root_dir_virtual_addr;
    for (int i = 0; i < root_dir_items; i++) {
        DIR_LFN_t* file_entry = (DIR_LFN_t*)root_dir_item_addr;
        printf("File %d: %.8s . %.3s | File size: %d bytes\n", i + 1, file_entry->info83.filename, file_entry->info83.file_extension, file_entry->info83.filesize);

        // Add info to the file array
        memcpy(files[i].filename, file_entry->info83.filename, 8);
        memcpy(files[i].file_extension, file_entry->info83.file_extension, 3);
        files[i].filesize = file_entry->info83.filesize;
        files[i].start_cluster = file_entry->info83.first_cluster_lo;

        // Iterate 64 bytes to next file entry
        root_dir_item_addr += 0x40;
    }
}

void fat16_read_file(uint16_t start_cluster, uint16_t filesize) {
    // Allocate memory for the file
    uintptr_t file_virtual_addr = 0x9010000000;
    vmmgr_kalloc_page(file_virtual_addr, PAGE_ALIGN_UP(filesize) / 4096);
    i386_kern_memset((uint8_t*)file_virtual_addr, 0x00, PAGE_ALIGN_UP(filesize) / 4096);
    // WOAH WOAH TODO HOLY FUCK IM NOT ZEROING THE PAGE CORRECTLY!!!! HOL UP TOO SMALL
    printf("[FAT16] Reading file of %d bytes to 0x%llx\n", filesize, file_virtual_addr);

    // Calculate what sector the file's data starts in
    int file_sector_start = data_start_sector + (start_cluster - 2) * bpb->sectors_per_cluster;

    // Calculate how many sectors to read in, and read the file
    for (size_t i = 0; i < DIV_ROUND_UP(filesize, 512); i++) {
        ata_pio_read(file_sector_start + i, file_virtual_addr + (0x200 * i));
    }
}
