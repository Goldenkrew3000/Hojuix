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
#include <fs/fat16.h>
#include <drivers/ata_pio.h>
#include <memory/pmmgr.h>
#include <memory/vmmgr.h>
#include <i386/asm_functions.h>

#define DIV_ROUND_UP(a, b) (((a) + (b) - 1) / (b))

BPB_t* bpb = NULL;
EBR_t* ebr = NULL;
int root_dir_start_sector_offset = 0;
int root_dir_size_sectors = 0;
int data_start_sector = 0;
int root_dir_items = 0;
fat16_file_t files[32]; // Support 32 files

int previous_file_pages = 0;

int fat16_strncmp(const char *s1, const char *s2, size_t n) // Not at all compliant strncmp implementation
{
    for (size_t i = 0; i < n; i++) {
        if (s1[i] != s2[i]) {
            return 1;
        }
    }
    return 0;
}

int fat16_strcmp(const char *s1, const char *s2)
{
    while (*s1 == *s2)
    {
        if (*s1 == '\0')
        {
            return 0;
        }

        ++s1;
        ++s2;
    }

    return *s1 - *s2;
}


void fat16_fs_test() {
    printf("[FAT16] Reading LBA 0 from disk.\n");

    uintptr_t lba0 = (uintptr_t)pmmgr_kmalloc(1); // 4KB for 512 bytes is wasteful, I know, but i havent got a slab alloc yet
    lba0 += 0xFFFF800000000000; // Add VMMGR Identity Map offset
    i386_memset((uint8_t*)lba0, 0x00, 4096);
    ata_pio_read(0, lba0);
    printf("[FAT16] Raw bootsector at %llx.\n", lba0);

    fat16_parse_bootsector(lba0);
    fat16_read_root_directory();

    // Find the file SHELL.ELF file (Files are spaced out to 8 characters for FAT16)
    for (size_t i = 0; i < root_dir_items; i++) {
        if (fat16_strncmp(files[i].filename, "SHELL   ", 8) == 0) {
            if (fat16_strncmp(files[i].file_extension, "ELF", 3) == 0) {
                fat16_read_file(files[i].start_cluster, files[i].filesize);
            }
        }
    }
}

void fat16_read_in_exec_file(char* filename) {
    printf("[FAT16] Finding %s for exec()\n", filename);

    // Search file tree for index
    for (size_t i = 0; i < root_dir_items; i++) {
        if (fat16_strcmp(files[i].filename_full, filename) == 0) {
            printf("[FAT16] Reading in %s for exec()\n", filename);
            fat16_read_file(files[i].start_cluster, files[i].filesize);
        }
    }
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
    i386_memset((uint8_t*)root_dir_virtual_addr, 0x00, root_dir_size_sectors * bpb->bytes_per_sector);

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

        // Make a full filename for finding files (This code is super cursed)
        // Add the filename
        int filename_full_index = 0;
        for (size_t j = 0; j < 8; j++) {
            if (file_entry->info83.filename[j] != 0x20) {
                files[i].filename_full[filename_full_index] = file_entry->info83.filename[j];
                filename_full_index++;
            }
        }
        // Add the '.'
        files[i].filename_full[filename_full_index] = '.';
        filename_full_index++;
        // Add the file extension
        for (size_t j = 0; j < 3; j++) {
            if (file_entry->info83.file_extension[j] != 0x20) {
                files[i].filename_full[filename_full_index] = file_entry->info83.file_extension[j];
                filename_full_index++;
            }
        }

        // Add info to the file array
        i386_memcpy(files[i].filename, file_entry->info83.filename, 8);
        i386_memcpy(files[i].file_extension, file_entry->info83.file_extension, 3);
        files[i].filesize = file_entry->info83.filesize;
        files[i].start_cluster = file_entry->info83.first_cluster_lo;

        // Iterate 64 bytes to next file entry
        root_dir_item_addr += 0x40;
    }
}

void fat16_read_file(uint16_t start_cluster, uint16_t filesize) {
    uintptr_t file_virtual_addr = 0x9010000000;

    // Attempt to free memory from previous file
    vmmgr_kfree_page(file_virtual_addr, previous_file_pages);

    // Allocate memory for the file
    vmmgr_kalloc_page(file_virtual_addr, PAGE_ALIGN_UP(filesize) / 4096);
    previous_file_pages = PAGE_ALIGN_UP(filesize) / 4096;
    i386_memset((uint8_t*)file_virtual_addr, 0x00, PAGE_ALIGN_UP(filesize) / 4096);
    // WOAH WOAH TODO HOLY FUCK IM NOT ZEROING THE PAGE CORRECTLY!!!! HOL UP TOO SMALL
    printf("[FAT16] Reading file of %d bytes to 0x%llx\n", filesize, file_virtual_addr);

    // Calculate what sector the file's data starts in
    int file_sector_start = data_start_sector + (start_cluster - 2) * bpb->sectors_per_cluster;

    // Calculate how many sectors to read in, and read the file
    for (size_t i = 0; i < DIV_ROUND_UP(filesize, 512); i++) {
        ata_pio_read(file_sector_start + i, file_virtual_addr + (0x200 * i));
    }
}
