#include <kern/kprintf.h>
#include <fs/ext2.h>
#include <drivers/nvme.h>
#include <memory/pmmgr.h>
#include <kernel.h>

#define DIV_ROUND_UP(a, b) (((a) + (b) - 1) / (b))

int ext2_init(uint64_t partition_offset) {
    // Read the superblock (LBA2)
    uintptr_t superblock_addr = pmmgr_kmalloc_contiguous(1) + 0xFFFF800000000000;
    memset((uint8_t*)superblock_addr, 0x00, 4096);
    nvme_read(superblock_addr, partition_offset + 2, 2); // Read in 1kb
    EXT2_Superblock_t* superblock = (EXT2_Superblock_t*)superblock_addr;

    // Check the EXT2 Magic
    if (superblock->s_magic != EXT2_MAGIC) {
        printf("[EXT2] Magic is not valid.\n");
        return -EINVAL;
    }

    // Get the block size
    uint32_t superblock_block_size = 1024 << superblock->s_log_block_size;

    // Print some basic information from the superblock
    printf("[EXT2] EXT2 Partition with Version %d.%d found.\n", superblock->s_major_rev_level, superblock->s_minor_rev_level);
    printf("[EXT2] Block size: %ld\n", superblock_block_size);
    printf("[EXT2] Filesystem State: ");
    switch (superblock->s_state) {
        case EXT2_FileSystemState_Clean:
            printf("Clean.\n");
            break;
        case EXT2_FileSystemState_Errors:
            printf("Errors.\n");
            break;
        default:
            printf("Unknown.\n");
            break;
    }
    printf("[EXT2] Error Handling Method: ");
    switch (superblock->s_errors) {
        case EXT2_ErrorHandlingMethod_Ignore:
            printf("Ignore.\n");
            break;
        case EXT2_ErrorHandlingMethod_RemountRO:
            printf("Remount Read-only.\n");
            break;
        case EXT2_ErrorHandlingMethod_KPanic:
            printf("Kernel Panic.\n");
            break;
        default:
            printf("Unknown.\n");
            break;
    }

    // Determine the number of block groups
    uint32_t block_group_count_a = DIV_ROUND_UP(superblock->s_blocks_count, superblock->s_blocks_per_group); // Round up the total number of blocks divided by the number of block per block group
    uint32_t block_group_count_b = DIV_ROUND_UP(superblock->s_inodes_count, superblock->s_inodes_per_group); // Round up the total number of inodes divided by the number of inodes per block group
    // TODO check against each other and something idk osdev wiki https://wiki.osdev.org/Ext2#Determining_the_Number_of_Block_Groups

    // Locate and read the Block Group Descriptor Table
    // If the block size is 1024 bytes, the table begins at block 2
    // Else, it begins at block 1
    // NOTE: Blocks are numbered starting at 0
    if (superblock_block_size == 1024) {
        // SPECIAL handling needed TODO
    }
    
#if AAA
    uintptr_t block_group_descriptor_table_addr = pmmgr_kmalloc_contiguous(1) + 0xFFFF800000000000; // TODO calc size
    memset((uint8_t*)block_group_descriptor_table_addr, 0x00, 4096);
    nvme_read(block_group_descriptor_table_addr, partition_offset + (superblock_block_size / 512), 1);
    EXT2_BlockGroupDescriptorTable_t* block_group_descriptor_table = (EXT2_BlockGroupDescriptorTable_t*)block_group_descriptor_table_addr;
    for (size_t i = 0; i < block_group_count_a; i++) { // CHANGE TODO
        //printf("Block start inode: %lx\n", block_group_descriptor_table->block_group_descriptor_array[i].bg_inode_table);
    }

    // TEST get inode
    int inode = 2; // Root dir inode
    /*
    uint32_t inode_table = block_group_descriptor_table->block_group_descriptor_array[0].bg_inode_table;
    uint32_t index_in_group = (inode - 1) % superblock->s_inodes_per_group;
    uint32_t inode_offset = index_in_group * sizeof(EXT2_InodeDataStructure_t);

    uint32_t inodes_per_block = superblock_block_size / sizeof(EXT2_InodeDataStructure_t);
    uint32_t target_block = inode_table + (inode_offset / superblock_block_size);
    uint32_t offset_in_block = inode_offset % superblock_block_size;*/
    printf("Inode size: %ld\n", superblock->s_inode_size);

    uint32_t block_group = (inode - 1) / superblock->s_inodes_per_group;
    uint32_t local_inode_index = (inode - 1) % superblock->s_inodes_per_group;
    
    // Offset of inode table in blocks
    uint32_t inode_table = block_group_descriptor_table->block_group_descriptor_array[block_group].bg_inode_table;


    // Read in block
    uintptr_t block_addr = pmmgr_kmalloc_contiguous(1) + 0xFFFF800000000000;
    memset((uint8_t*)block_addr, 0x00, 4096);
    nvme_read(block_addr, partition_offset + (inode_table * 8), 8); // Read the inode block

    //printf("Block addr: %lx\n", block_addr);
    //printf("Offset in block: %ld\n", offset_in_block);
    
    EXT2_InodeDataStructure_t* root_inode = (EXT2_InodeDataStructure_t*)(block_addr + (local_inode_index * superblock->s_inode_size));
    if ((root_inode->i_mode & 0xF000) != 0x4000) {
        printf("Root inode is not a directory!");
    } else { printf("yippee!\n"); }

    printf("Mode: %lx, Size: %lx\n", root_inode->i_mode, root_inode->i_size);

    // Read through the 12 block pointers
    uintptr_t block_b_addr = pmmgr_kmalloc_contiguous(1) + 0xFFFF800000000000;
    for (size_t i = 0; i < 12; i++) {
        memset((uint8_t*)block_b_addr, 0x00, 4096);
        nvme_read(block_b_addr, partition_offset + (root_inode->i_block[i] * 8), 8);
        EXT2_Directory_t* dir = (EXT2_Directory_t*)block_b_addr;

        // Skip null entries (unused)
        if (dir->inode != 0) {
            printf("found!\n");
            printf("Name: %s\n", dir->name);
        } 
    }

    uint8_t* aa = (uint8_t*)block_addr;
    for (size_t i = 0; i < 384; i++) {
        printf("%.2x ", aa[i]);
        if (i != 0 && i % 48 == 0) { printf("\n"); }
    }
#endif
}
