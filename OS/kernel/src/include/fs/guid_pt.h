#ifndef _GUID_PT_H
#define _GUID_PT_H
#include <stdint.h>

typedef struct {
    uint8_t bootable;   // 0x0 -> Not bootable (good), 0x1 -> Bootable (bad)
    uint8_t other[511]; // TODO finish
} __attribute__((packed)) GUID_PMBR_t; // Protective Master Boot Record (LBA 0)

typedef struct {
    uint8_t signature[8];   // Should be 'EFI PART'
    uint32_t gpt_revision;  // GPT Revision
    uint32_t hdr_size;      // Header size
    uint32_t hdr_checksum;  // CRC32 checksum of the GPT header (0x0 to 0x5c)
    uint32_t rsvd;
    uint64_t hdr_lba;       // The LBA containing this header
    uint64_t alt_hdr_lba;   // The LBA containing the alternate GPT header
    uint64_t first_usable_block;    // First usable block that can be contained in a GPT entry 
    uint64_t last_usable_block;     // Last usable block that can be contained in a GPT entry
    uint8_t guid[16];               // GUID of the disk
    uint64_t aa;
    uint32_t bb;
    uint32_t cc;
    uint32_t dd;
    uint32_t rsvd2[105];
} __attribute__((packed)) GUID_Partition_TblHdr_t; // GUID Partition Table Header (LBA 1)

// Note: GUID Formatting is AABBCCDD-AABB-AABB-AABB-AABBCCDDEEFF
// Each section is little endian, so the identifier is DDCCBBAA-BBAA etc...
// But still in that order : /
// GUIDs are fucked: https://en.wikipedia.org/wiki/GUID_Partition_Table#cite_note-GUID-Endian-14
typedef struct {
    uint8_t part_type_guid[16];
    uint8_t part_guid[16];
    uint64_t lba_start;
    uint64_t lba_end;
    uint64_t attributes;
    uint8_t name[72]; // might be utf16le...
} __attribute__((packed)) GUID_Partition_Entry_t;

typedef struct {
    GUID_PMBR_t pmbr;
    GUID_Partition_TblHdr_t part_tbl_hdr;
    GUID_Partition_Entry_t part_entries[128]; // Maximum of 128 partitions
} __attribute__((packed)) GUID_t;

uint64_t guid_pt_parse(uintptr_t guid_pt_addr);

#endif
