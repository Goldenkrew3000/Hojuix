#ifndef _EXT2_H
#define _EXT2_H
#include <stdint.h>

#define EXT2_MAGIC 0xEF53

/*
// Base EXT2 Superblock
*/
typedef struct { // The Second Extended File System Internal Layout - Page 7 (Table 3.3)
    /*
    // EXT2 Base Specification
    */
    uint32_t s_inodes_count;        // Total number of inodes in file system
    uint32_t s_blocks_count;        // Total number of blocks in a filesystem
    uint32_t s_r_blocks_count;      // Number of blocks reserved for superuser
    uint32_t s_free_blocks_count;   // Total number of unallocated blocks
    uint32_t s_free_inodes_count;   // Total number of unallocated inodes
    uint32_t s_first_data_block;    // Block number of the block containing the superblock
    uint32_t s_log_block_size;      // Log2 (Block Size) - 10
    uint32_t s_log_frag_size;       // Log2 (Fragment Size) - 10
    uint32_t s_blocks_per_group;    // Number of blocks in each block group
    uint32_t s_frags_per_group;     // Number of fragments om each block group
    uint32_t s_inodes_per_group;    // Number of inodes in each block group
    uint32_t s_mtime;               // Last mount time (in POSIX time)
    uint32_t s_wtime;               // Last written time (in POSIX time)
    uint16_t s_mnt_count;           // Number of times the volume has been mounted since it's last consistency check
    uint16_t s_max_mnt_count;       // Number of mounts allowed before a consistency check must be done
    uint16_t s_magic;               // EXT2 signature (0xEF53)
    uint16_t s_state;               // File system state
    uint16_t s_errors;              // What to do when an error is detected
    uint16_t s_minor_rev_level;     // Minor portion of version
    uint32_t s_lastcheck;           // POSIX time of the last consistency check
    uint32_t s_checkinterval;       // Interval (in POSIX time) between forced consistency checks
    uint32_t s_creator_os;          // Operating System ID from which the filesystem on this volume was created
    uint32_t s_major_rev_level;     // Major portion of version
    uint16_t s_def_resuid;          // User ID that can use reserved blocks
    uint16_t s_def_resgid;          // Group ID that can use reserved blocks
    /*
    // EXT2_DYNAMIC_REV Extension
    */
    uint32_t s_first_ino;
    uint16_t s_inode_size;
    uint16_t s_block_group_nr;
    uint32_t s_feature_compat;
    uint32_t s_feature_incompat;
    uint32_t s_feature_ro_compat;
    uint8_t s_uuid[16];
    uint8_t s_volume_name[16];
    uint8_t s_last_mounted[64];
    uint32_t s_algo_bitmap;
    /*
    // Performance Hints
    */
    uint8_t s_prealloc_blocks;
    uint8_t s_prealloc_dir_blocks;
    uint16_t rsvd_alignment;
    /*
    // Journaling Support
    */
    uint8_t s_journal_uuid[16];
    uint32_t s_journal_inum;
    uint32_t s_journal_dev;
    uint32_t s_last_orphan;
    /*
    // Directory Indexing Support
    */
    uint32_t s_hash_seed[4];
    uint8_t s_def_hash_version;
    uint8_t rsvd_padding[3];
    /*
    // Other Options
    */
    uint32_t s_default_mount_options;
    uint32_t s_first_meta_bg;
} __attribute__((packed)) EXT2_Superblock_t; // Located at byte 1024, length of 1024 (LBA2 @ 512 bytes / sector)

#define EXT2_FileSystemState_Clean  1           // File system is clean
#define EXT2_FileSystemState_Errors 2           // File system has errors
#define EXT2_ErrorHandlingMethod_Ignore 1       // Ignore the error
#define EXT2_ErrorHandlingMethod_RemountRO 2    // Remount the file system as RO
#define EXT2_ErrorHandlingMethod_KPanic 3       // Kernel Panic

enum EXT2_ErrorHandlingMethod_e {
    ErrorHandlingMethod_Ignore = 1,
    ErrorHandlingMethod_RemountRO = 2,
    ErrorHandlingMethod_KPanic = 3
};

enum EXT2_CreatorOperatingSystemID_e {
    CreatorOperatingSystem_Linux = 0,
    CreatorOperatingSystem_GnuHurd = 1,
    CreatorOperatingSystem_Masix = 2,
    CreatorOperatingSystem_FreeBSD = 3,
    CreatorOperatingSystem_OtherLites = 4
};

typedef struct {  // The Second Extended File System Internal Layout - Page 16 (Table 3.12)
    uint32_t bg_block_bitmap;       // Block address of the block usage bitmap
    uint32_t bg_inode_bitmap;       // Block address of the inode usage bitmap
    uint32_t bg_inode_table;        // Starting block address of the inode table
    uint16_t bg_free_blocks_count;  // Number of unallocated blocks in the group
    uint16_t bg_free_inodes_count;  // Number of unallocated inodes in the group
    uint16_t bg_used_dirs_count;    // Number of directories in the group
    uint16_t bg_pad;
    uint8_t rsvd[12];
} __attribute__((packed)) EXT2_BlockGroupDescriptor_t;

typedef struct {
    EXT2_BlockGroupDescriptor_t block_group_descriptor_array[];
} __attribute__((packed)) EXT2_BlockGroupDescriptorTable_t;

typedef struct { // The Second Extended File System Internal Layout - Page 18 (Table 3.13)
    uint16_t i_mode;            // Type and permissions
    uint16_t i_uid;             // User ID
    uint32_t i_size;            // Lower 32 bits of size in bytes
    uint32_t i_atime;           // Last access time (in POSIX time)
    uint32_t i_ctime;           // Creation time (in POSIX time)
    uint32_t i_mtime;           // Last modification time (in POSIX time)
    uint32_t i_dtime;           // Deletion time (in POSIX time)
    uint16_t i_gid;             // Group ID
    uint16_t i_links_count;     // --
    uint32_t i_blocks;          // --
    uint32_t i_flags;           // Flags
    uint32_t i_osd1;            // Operating System Specific Value 1
    uint32_t i_block[15];       // Direct Block Pointer 0-11, Singly Indir TODO -------
    uint32_t i_generation;      // Generation number (Mostly used for NFS)
    uint32_t i_file_acl;        // --
    uint32_t i_dir_acl;         // --
    uint32_t i_faddr;           // Block address of fragment
    uint8_t i_osd2[12];         // Operating System Specific Value 2
} __attribute__((packed)) EXT2_InodeDataStructure_t;

typedef struct { // The Second Extended File System Internal Layout - Page 26 (Table 4.1)
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    uint8_t name[255]; // HUH TODO CHECK OUT
} __attribute__((packed)) EXT2_Directory_t;

int ext2_init(uint64_t partition_offset);

#endif
