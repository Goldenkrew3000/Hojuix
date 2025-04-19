#ifndef _FAT16_H
#define _FAT16_H
#include <stdint.h>

typedef struct {
    uint8_t padding1[3];                // Padding
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;          // Reserved sectors (including boot record/bpb)
    uint8_t fat_count;                  // Number of FATs on the storage media
    uint16_t root_dir_entries;
    uint16_t total_sectors;             // 특별한 아이템
    uint8_t media_descriptor_type;
    uint16_t sectors_per_fat;           // Number of sectors per FAT (FAT12/16만)
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t number_of_hidden_sectors;
    uint32_t large_sector_count;        // ---- 특별
} __attribute__((packed)) BPB_t; // BIOS Protection Block

typedef struct {
    uint8_t drive_number;
    uint8_t nt_flags;                   // Windows NT Flags, not applicable here (obviously)
    uint8_t signature;                  // Signature MUST be 0x28 or 0x29
    uint32_t volume_id;                 // Volume ID / Serial number
    uint8_t volume_label[11];           // Volume label string (padded with spaces)
    uint8_t system_identifier[8];       // System identifier string (String representation of FAT type, padded with spaces)
    uint32_t padding[112];              // Boot code, not applicable, 448 bytes long
    uint16_t boot_signature;
} __attribute__((packed)) EBR_t; // Extended Boot Record (FAT16 compatible)

typedef struct {
    uint8_t filename[8];                    //
    uint8_t file_extension[3];              //
    uint8_t attributes;                     //
    uint8_t nt_reserved;
    uint8_t create_time_hundreth_second;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t last_accessed;
    uint16_t first_cluster_hi;
    uint16_t last_modify_time;
    uint16_t last_modify_date;
    uint16_t first_cluster_lo;
    uint32_t filesize;
} __attribute__((packed)) DIR_83_t; // FAT12/16/32 Directory (Standard 8.3 version)

typedef struct {
    uint8_t order;
    uint8_t info[31];

    DIR_83_t info83;
} __attribute__((packed)) DIR_LFN_t; // FAT12/16/32 Directory (Long filename version)

void fat16_fs_test();
void fat16_parse_bootsector();

#endif
