/*
// Hojuix GUID Partition Table / GPT Parser
// 2025-05-02
// GPLv3
*/

#include <stdio.h>
#include <fs/guid_pt.h>

// Format: AABBCCDD-AABB-AABB-AABB-AABBCCDDEEFF
//         28732ac1-1ff8-d211-ba4b-00a0c93ec93b

//
void guid_pt_parse(uintptr_t guid_pt_addr) {
    printf("[GUID_PT] Parsing GUID Partition Table...\n");

    GUID_t* guid_pt = (GUID_t*)guid_pt_addr;
    printf("EFI: %s\n", guid_pt->part_tbl_hdr.signature);

    for (size_t part = 0; part < 128; part++) {
        if (guid_pt->part_entries[part].part_type_guid[0] != 0x00) {
            printf("Part %d type: ", part);
            printf("%.2X%.2X%.2X%.2X-%.2X%.2X-%.2X%.2X-%.2X%.2X-%.2X%.2X%.2X%.2X%.2X%.2X\n",
            guid_pt->part_entries[part].part_type_guid[3],
            guid_pt->part_entries[part].part_type_guid[2],
            guid_pt->part_entries[part].part_type_guid[1],
            guid_pt->part_entries[part].part_type_guid[0],
            guid_pt->part_entries[part].part_type_guid[5],
            guid_pt->part_entries[part].part_type_guid[4],
            guid_pt->part_entries[part].part_type_guid[7],
            guid_pt->part_entries[part].part_type_guid[6],
            guid_pt->part_entries[part].part_type_guid[8],
            guid_pt->part_entries[part].part_type_guid[9],
            guid_pt->part_entries[part].part_type_guid[10],
            guid_pt->part_entries[part].part_type_guid[11],
            guid_pt->part_entries[part].part_type_guid[12],
            guid_pt->part_entries[part].part_type_guid[13],
            guid_pt->part_entries[part].part_type_guid[14],
            guid_pt->part_entries[part].part_type_guid[15]);

            printf("Part %d UUID: ", part);
            printf("%.2X%.2X%.2X%.2X-%.2X%.2X-%.2X%.2X-%.2X%.2X-%.2X%.2X%.2X%.2X%.2X%.2X\n",
            guid_pt->part_entries[part].part_guid[3],
            guid_pt->part_entries[part].part_guid[2],
            guid_pt->part_entries[part].part_guid[1],
            guid_pt->part_entries[part].part_guid[0],
            guid_pt->part_entries[part].part_guid[5],
            guid_pt->part_entries[part].part_guid[4],
            guid_pt->part_entries[part].part_guid[7],
            guid_pt->part_entries[part].part_guid[6],
            guid_pt->part_entries[part].part_guid[8],
            guid_pt->part_entries[part].part_guid[9],
            guid_pt->part_entries[part].part_guid[10],
            guid_pt->part_entries[part].part_guid[11],
            guid_pt->part_entries[part].part_guid[12],
            guid_pt->part_entries[part].part_guid[13],
            guid_pt->part_entries[part].part_guid[14],
            guid_pt->part_entries[part].part_guid[15]);
        }
    }
}