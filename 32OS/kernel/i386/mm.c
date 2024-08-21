#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kernel/mm.h>
#include <kernel_ext/multiboot.h>

/*
// Multiboot Memory Map Handler
*/
#define MMMH_MEMCAP 3758096384 // 3.5GB
#define MMMH_MEMCAP_ALLOC 3757572096 // MMMH_MEMCAP - 512kb
uint64_t mmmh_memSize = 0;

void init_mmmh(multiboot_info_t* mbd, uint32_t magic) {
    // Calculate initial memory size (Can go over 4GB)
    for (int i = 0; i < mbd->mmap_length; i += sizeof(multiboot_memory_map_t)) {
        multiboot_memory_map_t* mmmt = (multiboot_memory_map_t*) (mbd->mmap_addr + i);
        
        // Pull data from the multiboot struct
        uint64_t length = mmmt->len;

        //uint32_t length_low = mmmt->len_low;
        //uint32_t length_high = mmmt->len_high;

        // Calculate memory size
        mmmh_memSize += length;
        //mmmh_memSize += length_low + (length_high << 32);
    }
    printf("Initial memory size: %ld kb\n", (mmmh_memSize / 1024));

    // Check if detected memory is over 3.5GB (Hard cap)
    if (mmmh_memSize > MMMH_MEMCAP) {
        // Too much memory to fully address on a 32 bit kernel, set hard cap
        mmmh_memSize = MMMH_MEMCAP;
    }

    // Calculate kernel size (Required for placing the PPM Bitmap)
    uint32_t kernsize = &endkernel - &startkernel;
    printf("Kernel size in memory: 0x%x (", kernsize);
    printf("0x%x - ", &startkernel);
    printf("0x%x)\n", &endkernel);

    // Init the PMM (Done here due to needing to do more multiboot stuff afterwards)
    printf("Initializing PMM...");
    //pmmgr_init((mmmh_memSize / 1024), (uint32_t)0x20B000); // PMM takes initial memory size in kb, And the 0x1 gives it one byte of padding from kernel BSS to PMM Bitmap
    pmmgr_init(mmmh_memSize, (uint32_t)0x20B000);
    printf(" OK\n");
    

    // NOTE: Next aligned 4096 block after kernel space (not used) is 0x20B000
    // Testing this is hard coded, but write function to get space after kernel
    // and align to next 4096 aligned block, blacklist kernel start to the start of it - 1

    // But also have to deinit 0x20B00 - 0x20BFFF due to it holding the PMM bitmap

    // Add the multiboot memory map to the PMM
    // FIX - Leave 512kb for PMM padding (Without it, it does not work), Held in MMMH_MEMCAP_ALLOC
    uint64_t mmmh_InitMemSize = 0;
    uint64_t mmmh_difference = 0;
    for (int i = 0; i < mbd->mmap_length; i += sizeof(multiboot_memory_map_t)) {
        // Pull data from the multiboot struct
        multiboot_memory_map_t* mmmt = (multiboot_memory_map_t*) (mbd->mmap_addr + i);
        uint32_t start_addr = mmmt->addr;
        uint64_t length = mmmt->len;
        uint32_t type = mmmt->type;

        // Check if the memory is usable
        // Type 1 --> Available, Type 2 --> Reserved, Type 0x3 --> ACPI Reserved
        if (type == 1) {
            // Check if adding the new memory region would make the memory amount go over 3.5GB
            int64_t mmmh_memorySizeCheck = mmmh_InitMemSize + length;
            if (mmmh_memorySizeCheck > MMMH_MEMCAP_ALLOC) {
                // It would, calculate the difference
                printf("WARN: Too much memory!\n");
                mmmh_difference = MMMH_MEMCAP_ALLOC - mmmh_InitMemSize;

                // Only initialize the difference
                mmmh_InitMemSize += mmmh_difference;
                pmmgr_init_region(start_addr, mmmh_difference);
                break;
            } else {
                // It wouldn't, initialize it all
                mmmh_InitMemSize += length;
                pmmgr_init_region(start_addr, length);
            }
        }
    }

    // Deinit (Blacklist) Kernel Region + 1mb (PMM Bitmap held there)
    uint32_t kern_blacklist_len = (0x20C000 - 1) - (uint32_t)&startkernel;
    //printf("START ADDR: 0x%x\n", (uint32_t)&startkernel);
    //printf("LEN: 0x%x\n", kern_blacklist_len);
    //printf("START GOOD: 0x%x\n", (uint32_t)&startkernel + kern_blacklist_len+1);
    //pmmgr_deinit_region((uint32_t)&startkernel, kern_blacklist_len);

    // Include PMM padding in the difference calculation (At this point, it is only used for printing to console)
    mmmh_difference += MMMH_MEMCAP - MMMH_MEMCAP_ALLOC;
    printf("Usable memory size: %d kb (", (mmmh_InitMemSize / 1024));
    printf("0x%x) (diff: ", mmmh_InitMemSize);
    printf("0x%x)\n", mmmh_difference);

    // Finally, print the memory map to the console TODO
    // TODO: Print a flags column to say if fully in use (PF <-- PMM Full), or half in use (PH <-- PMM Half)
    //       Would have to scan the whole memory map though*
}

/*
// PMM
*/

// Potential issues with the PMM
// Will happily allocate to overwrite kernel space
// Taking it over 4GB fucks it up
// NOTE: GDT and IDT seem to move out of the way... not sure though. Would like to move allocation spot past GDT/IDT

// P.S. Replaced all physical_addr with uint32_t (TODO)

/*
// PMM Logic for usage with systems over 4GB (2^32 bytes) memory.
// If the system has over the 32 bit limit of memory, the PMM cannot calculate the blocks correctly
// (Flipping to negative) which causes a whole lot of issues.
// The way I have chose to fix this is to set a hard limit at ~3.5GB.
// The problem is that generally you just init a whole block of memory from the bootloader, but what
// if the 3.5GB mark ends in the middle of a block? Are we just going to miss out on free usable memory?
// NO!!! We calculate the difference and only allocate half the usable chunk. Thus giving us the most memory possible :)
*/

// http://www.brokenthorn.com/Resources/OSDev17.html
/*
#define PMMGR_BLOCKS_PER_BYTE 8 // 8 blocks per byte
#define PMMGR_BLOCK_SIZE 4096 // 4k block size
#define PMMGR_BLOCK_ALIGN PMM_BLOCK_SIZE // Block alignment

static uint32_t mmgr_memory_size = 0; // Size of physical memory
static uint32_t mmgr_used_blocks = 0; // Number of blocks currently in use
static uint32_t mmgr_max_blocks = 0; // Maximum number of available memory blocks
static uint32_t* mmgr_memory_map = 0; // Memory map bit array. Each bit represents a memory block

void mmap_set(int bit) { // Was inline
    mmgr_memory_map[bit / 32] |= (1 << (bit % 32));
}

void mmap_unset(int bit) { // Was inline
    mmgr_memory_map[bit / 32] &= ~ (1 << (bit % 32));
}

bool mmap_test(int bit) { // Was inline
    return mmgr_memory_map[bit / 32] & (1 << (bit % 32));
}

int mmap_first_free() {
    // Find the first free bit
    for (uint32_t i = 0; i < pmmgr_get_block_count(); i++) { // CHANGED
        if (mmgr_memory_map[i] != 0xffffffff) {
            // Test each bit in the DWORD
            for (int j = 0; j < 32; j++) {
                int bit = 1 << j;
                if (!(mmgr_memory_map[i] & bit)) {
                    return i * 4 * 8 + j;
                }
            }
        }
    }
    return -1;
}
*/

/*
int mmap_first_free_s(size_t size) {
    // TODO THIS IS A COPY, ACTUALLY IMPLEMENT
    	if (size==0)
		return -1;

	if (size==1)
		return mmap_first_free ();

	for (uint32_t i=0; i<pmmgr_get_block_count() /32; i++)
		if (mmgr_memory_map[i] != 0xffffffff)
			for (int j=0; j<32; j++) {	//! test each bit in the dword

				int bit = 1<<j;
				if (! (mmgr_memory_map[i] & bit) ) {

					int startingBit = i*32;
					startingBit+=bit;		//get the free bit in the dword at index i

					uint32_t free=0; //loop through each bit to see if its enough space
					for (uint32_t count=0; count<=size;count++) {

						if (! mmap_test (startingBit+count) )
							free++;	// this bit is clear (free frame)

						if (free==size)
							return i*4*8+j; //free count==size needed; return index
					}
				}
			}

	return -1;
}
*/

/*
void pmmgr_init(uint32_t memSize, uint32_t bitmap_addr) {
    mmgr_memory_size = memSize; // Memory size in kb
    mmgr_memory_map = (uint32_t*)bitmap_addr; // Address of bitmap in memory (Right after kernel)
    mmgr_max_blocks = (pmmgr_get_memory_size() * 1024) / PMMGR_BLOCK_SIZE; // Memory in bytes / 4kb
    mmgr_used_blocks = pmmgr_get_block_count();

    // By default, all memory is in use
    memset(mmgr_memory_map, 0xF, pmmgr_get_block_count() / PMMGR_BLOCKS_PER_BYTE);
}

void pmmgr_init_region(physical_addr base, size_t size) {
    int align = base / PMMGR_BLOCK_SIZE;
    int blocks = size / PMMGR_BLOCK_SIZE;

    for (; blocks > 0; blocks--) {
        mmap_unset(align++);
        mmgr_used_blocks--;
    }

    // First block is always set.
    mmap_set(0);
}

void pmmgr_deinit_region(physical_addr base, size_t size) {
    printf("Deinit base: 0x%x\n", base);
    printf("Size: 0x%x\n", size);
    int align = base / PMMGR_BLOCK_SIZE;
    int blocks = size / PMMGR_BLOCK_SIZE;

    for (; blocks > 0; blocks--) {
        mmap_set(align++);
        mmgr_used_blocks++;
    }
}

void* pmmgr_alloc_block() {
    if (pmmgr_get_free_block_count() <= 0) {
        // Out of memory
        return 0;
    }

    int frame = mmap_first_free();

    if (frame == -1) {
        // Out of memory
        return 0;
    }

    mmap_set(frame);

    physical_addr addr = frame * PMMGR_BLOCK_SIZE;
    mmgr_used_blocks++;

    return (void*)addr;
}
*/

/*
void* pmmgr_alloc_blocks(size_t size) {
    if (pmmgr_get_free_block_count() <= size) {
        // Not enough space
        return 0;
    }

    int frame = mmap_first_free_s(size);

    if (frame == -1) {
        // Not enough space
        return 0;
    }

    for (uint32_t i = 0; i < size; i++) {
        mmap_set(frame + i);
    }

    physical_addr addr = frame * PMMGR_BLOCK_SIZE;
    mmgr_used_blocks += size;

    return (void*)addr;
}
*/

/*
void pmmgr_free_block(void* p) {
    physical_addr addr = (physical_addr)p;
    int frame = addr / PMMGR_BLOCK_SIZE;

    mmap_unset(frame);

    mmgr_used_blocks--;
}
*/

/*
void pmmgr_free_blocks(void* p, size_t size) {
    physical_addr addr = (physical_addr)p;
    int frame = addr / PMMGR_BLOCK_SIZE;

    for (uint32_t i = 0; i < size; i++) {
        mmap_unset(frame + i);
    }

    mmgr_used_blocks -= size;
}
*/

/*
void* pmmgr_blacklist_blocks(uint32_t start_addr, size_t size) {
    if (pmmgr_get_free_block_count() <= 0) {
        // Out of memory
        return 0;
    }

    int frame = ;
}*/

/*
size_t pmmgr_get_memory_size() {
    return mmgr_memory_size;
}

uint32_t pmmgr_get_block_count() {
    return mmgr_max_blocks;
}

uint32_t pmmgr_get_use_block_count() {
    return mmgr_used_blocks;
}

uint32_t pmmgr_get_free_block_count() {
    return mmgr_max_blocks - mmgr_used_blocks;
}

uint32_t pmmgr_get_block_size() {
    return PMMGR_BLOCK_SIZE;
}
*/

//extern uint32_t endkernel; // Declared in the linker
//extern void load_page_directory();

// Page frame allocator needed first
// https://wiki.osdev.org/Writing_A_Page_Frame_Allocator

// Then https://wiki.osdev.org/Setting_Up_Paging
// ALSO THIS http://www.osdever.net/tutorials/view/implementing-basic-paging
// FROM http://www.osdever.net/tutorials/




/*
// PMM
*/
// Loosely based off of Brokenthorne's PMM, but I just could not get their's to work. So I wrote my own.
// It's less efficient, but at least it actually works.
/*
// SYSTEM MEMORY MAP
// 0x0 --> End of PMM Bitmap block is UNUSABLE
// 0x200000 --> Kernel Start
// GDT and IDT are in Kernel BSS
// Kernel End -> Next aligned 4096 (Such as 0x20B000) --> PMM Bitmap
*/
#define PMMGR_BLOCK_SIZE 4096 // 4k block size
#define PMMGR_PAGES_PER_BYTE 8 // 8 pages per byte (1 bit each)

uint32_t pmmgr_bitmap_start = 0;
uint32_t pmmgr_bitmap_end = 0;
uint32_t pmmgr_bitmap_aligned_end = 0;

uint32_t pmmgr_total_bitmap_pages = 0;
uint32_t pmmgr_used_bitmap_pages = 0;
uint32_t pmmgr_free_bitmap_pages = 0;


void pmmgr_init(uint32_t mem_size, uint32_t bitmap_addr) { // Feed mem_size ALL (All types) memory, in bytes, and bitmap_addr 0x20C000
    // Calculate bitmap page count.
    pmmgr_total_bitmap_pages = mem_size / 4096; // Maximum is 917504. TODO Check if multiple of 4 (somehow) here or before here
    printf("Bitmap Pages: %ld\n", pmmgr_total_bitmap_pages);

    // Calculate bitmap address offsets
    pmmgr_bitmap_start = bitmap_addr; // TESTING 0x20B000
    pmmgr_bitmap_end = pmmgr_bitmap_start + abs(pmmgr_total_bitmap_pages / PMMGR_PAGES_PER_BYTE);
    pmmgr_bitmap_aligned_end = pmmgr_align(pmmgr_bitmap_end);
    printf("Bitmap Start Addr: 0x%x\n", pmmgr_bitmap_start);
    printf("Bitmap End Addr: 0x%x\n", pmmgr_bitmap_end);
    printf("Bitmap Aligned End Addr: 0x%x\n", pmmgr_bitmap_aligned_end);

    // Create bitmap (Set all to 0xFF, Indicating USED / Not usable)
    memset((uint32_t*)pmmgr_bitmap_start, 0xFF, abs(pmmgr_total_bitmap_pages / PMMGR_PAGES_PER_BYTE)); // TODO Does not seem to set last bit? More testing required

    // THIS WORKS
    // 28th block (4th address space (8*3) + 3rd bit (0-3))
    // 20B000 + 0x3 --> 20B003 (0, 1, 2, 3)
    //pmmgr_set_bit(pmmgr_bitmap_start + 0x3, 5);

    // TODO Keep track of pages used. Like how many are used and stuff

    //pmmgr_first_free();
    
    //pmmgr_init_region(0x2FF900, 39938);
    //pmmgr_init_region(0x300000, 1073741824); // 0x400000, 27.58mb 28928972
    //pmmgr_init_region(0x0, 1048576);

    //pmmgr_init_region(0x300000, 1610612736);

    // 0x8000 --> 32768
    
    printf("COMPLETE\n");
}

uint32_t pmmgr_first_free() { // Returns address of first found free block, 0 if no space left (0 is not possible due to the artificial memory map)
    // Find the first free bit in the bitmap
    for (uint32_t i = 0; i < (pmmgr_total_bitmap_pages / PMMGR_PAGES_PER_BYTE); i++) {
        if (*((uint8_t*)pmmgr_bitmap_start + i) != 0xFF) {
            // At least one page in the page block (8 pages) is in use
            //printf("FOUND USE at page block %ld\n", i * 8);
            //return;

            // Check bits to know which block to send back for use
            // TODO: If statements are slow af, supposedly
            printf("0x%x\n", *((uint8_t*)pmmgr_bitmap_start + i));
            if (!(*((uint8_t*)pmmgr_bitmap_start + i) & (1 << 0))) {
                // Bit 0
                printf("It's bit 0\n");
            } else if (!(*((uint8_t*)pmmgr_bitmap_start + i) & (1 << 1))) {
                // Bit 1
                printf("It's bit 1\n");
            } else if (!(*((uint8_t*)pmmgr_bitmap_start + i) & (1 << 2))) {
                // Bit 2
                printf("It's bit 2\n");
            } else if (!(*((uint8_t*)pmmgr_bitmap_start + i) & (1 << 3))) {
                // Bit 3
                printf("It's bit 3\n");
            } else if (!(*((uint8_t*)pmmgr_bitmap_start + i) & (1 << 4))) {
                // Bit 4
                printf("It's bit 4\n");
            } else if (!(*((uint8_t*)pmmgr_bitmap_start + i) & (1 << 5))) {
                // Bit 5
                printf("It's bit 5\n");
            } else if (!(*((uint8_t*)pmmgr_bitmap_start + i) & (1 << 6))) {
                // Bit 6
                printf("It's bit 6\n");
            } else if (!(*((uint8_t*)pmmgr_bitmap_start + i) & (1 << 7))) {
                // Bit 7
                printf("It's bit 7\n");
            }
        }
    }

    //*((uint8_t*)pmmgr_bitmap_start + i) & (1 << 0)
    //

    printf("Could not find!\n");
    return 0x0;
}

void pmmgr_init_region(uint32_t addr, uint32_t size) {
    // NOTE: Tested up to 1536mb in a single allocation.
    // Check if size is over 8192 bytes (Guaranteed to get at least 1 page)
    if (size < 8192) {
        return;
    }

    // Find the first 4096 aligned block in the address space
    uint32_t aligned_addr = pmmgr_align(addr);

    // Find how many 4096 aligned blocks can be allocated from this address space
    uint32_t availableBlocks = abs((size - (aligned_addr - addr)) / 4096);

    // Give actual memory addresses for the init'd region
    printf("Allocated region at 0x%x - ", aligned_addr);
    printf("0x%x (", aligned_addr + (availableBlocks * 4096));
    printf("%ld blocks)\n", availableBlocks);

    for (uint32_t i = 0; i < availableBlocks; i++) {
        pmmgr_set_bit(
            abs((((aligned_addr + (i * 4096)) / 0x1000) / 8) + pmmgr_bitmap_start), // Position of real memory byte inside of the bitmap array
            (((aligned_addr + (i * 4096)) / 0x1000) % 8) // Bit position at byte
            );
    }

    return;
}

void pmmgr_deinit_region(uint32_t addr, uint32_t size) {
    //
}

// Bitmap set function
void pmmgr_set_bit(uint32_t addr, int bit) {
    // Set the bit to 0 (Usable by the PMM to offer for allocation)
    *((uint8_t*)addr) &= ~(1 << (bit % 8));
    pmmgr_used_bitmap_pages--;
    pmmgr_free_bitmap_pages++;
}

// Bitmap unset function
void pmmgr_unset_bit(uint32_t addr, int bit) {
    // Set the bit to 1 (Taken / Reserved)
    *((uint8_t*)addr) |= (1 << (bit % 8));
    pmmgr_used_bitmap_pages++;
    pmmgr_free_bitmap_pages--;
}

// Find first free block here

uint32_t pmmgr_align(uint32_t addr) {
    // Find the next aligned address
    if (addr % PMMGR_BLOCK_SIZE == 0) {
        // Address already aligned
        return addr;
    }

    // Round address up to alignment
    return (addr + PMMGR_BLOCK_SIZE - 1) & ~(PMMGR_BLOCK_SIZE - 1);
}





/*
// VMM
*/
