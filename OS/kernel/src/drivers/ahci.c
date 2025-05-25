/*
// Hojuix AHCI Driver
// 2025-04-30
// GPLv3
*/

#include <kern/kprintf.h>
#include <kern/libkern.h>
#include <drivers/ahci.h>
#include <drivers/pci.h>
#include <memory/vmmgr.h>
#include <memory/pmmgr.h>
#include <arch/amd64/asm_functions.h>
#include <drivers/i386/pit_timer.h>

hba_mem_t* hba_mem;
hba_capabilities_t* hba_capabilities;

#define ATA_CMD_READ_DMA_EX 0x25
#define FIS_TYPE_REG_H2D 0x27
#define HBA_PxIS_TFES (1 << 30)
#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08
#define AHCI_START 0x12340000

int ahci_init(uintptr_t bar_tbl_addr) {
    // Get the PCI BAR Table
    pci_device_bar_table_t* pci_bar_table = (pci_device_bar_table_t*)bar_tbl_addr;

    /*
    // PCI Initialization
    */
    // Enable interrupts, DMA, and memory space access in the PCI Command Register (Bit 1, 2, 10 is off)
    uint16_t pci_command_register = pci_readWord(pci_bar_table->bus, pci_bar_table->slot, pci_bar_table->func, PCI_OFFSET_COMMAND);
    pci_command_register &= ~(0x1 << 9);
    pci_command_register |= (0x3 << 1);
    pci_writeWord(pci_bar_table->bus, pci_bar_table->slot, pci_bar_table->func, PCI_OFFSET_COMMAND, pci_command_register);

    /*
    // MMIO Map BAR5 and ACHI memory
    */
    // Map BAR5 register as uncacheable
    // NOTE: Intentionally not memsetting this page, because I am not sure how that would work with MMIO
    vmmgr_mmio_map_uncache(pci_bar_table->bar5_addr, pci_bar_table->bar5_addr, 2); // With 32 ports, the space requirements are a little over a single page, so map 2
    
    // Map AHCI Memory also as uncacheable
    vmmgr_mmio_map_uncache(0x12340000, 0x12340000, 512); // TODO Make sure this is contiguous, and add a proper size
    i386_memset((uint8_t*)0x12340000, 0x00, 512 * 4096);

    // Get the HBA memory
    hba_mem = (hba_mem_t*)pci_bar_table->bar5_addr;

    /*
    // AHCI HBA Initialization
    */
    // Check if the device is capable of BIOS/OS Handoff. If yes, make sure we have ownership, if not, take ownership.
    if (hba_mem->cap2 & 0x1) {
        printf("[AHCI] Device is capable of BIOS/OS Handoff.\n");
        
        // Perform Handoff
        if (!(hba_mem->bohc & 0x2) && (hba_mem->bohc & 0x1)) {
            hba_mem->bohc |= 0x2;

            uint32_t spin = 0;
            while (hba_mem->bohc & 0x1) {
                if (spin > 1000000) {
                    printf("[AHCI] BIOS/OS handoff was not successful...\n");
                    return EXIT_FAILURE;
                } else {
                    spin++;
                }
            }
        }
    }

    // Make sure interrupts are disabled on the GHC
    hba_mem->ghc &= ~(1 << 1);

    // Register IRQ - TODO
    // On my Ryzen 7000 system, it wont assign an IRQ to the PCI, but in an emulator it does, so just use polling until APIC

    // Enable AHCI Mode on the GHC
    hba_mem->ghc |= (0x80000000);

    // Check for present SATA ports and perform a Reset / Start on them
    for (size_t port = 0; port < AHCI_MAX_PORTS; port++) {
        if (hba_mem->pi & (0x1 << port)) {
            hba_mem->ports[port].sctl = (hba_mem->ports[port].sctl & ~0xF) | 0x1; // Issue COMRESET (DET=1)
            timer_wait(20);
            hba_mem->ports[port].sctl &= ~0xF; // Clear DET
            timer_wait(20);
            hba_mem->ports[port].cmd |= (1 << 4); // Enable FIS
            hba_mem->ports[port].cmd |= (1 << 0);  // Start port
            timer_wait(20);
        }
    }

    // Now enable interrupts
    hba_mem->ghc |= (0x80000000 | 0x2);

    // Rebase the AHCI memory
    for (uint8_t port = 0; port < AHCI_MAX_PORTS; port++) {
        if (hba_mem->pi & (0x1 << port)) {
            // Issue a stop command
            hba_mem->ports[port].cmd &= ~0x1;
            hba_mem->ports[port].cmd &= ~0x10;
            
            // Wait until FR and CR are cleared
            while ((hba_mem->ports[port].cmd & 0x4000) || (hba_mem->ports[port].cmd & 0x8000)) {}

            // Rebase
            // Command headers. 32 headers * 32 bytes = 1kb per port / 32kb total
            hba_mem->ports[port].clb = AHCI_START + (port << 10);

            // FIS. 256 bytes * 32 ports = 8kb total
            hba_mem->ports[port].fb = AHCI_START + (AHCI_MAX_PORTS << 10) + (port << 8);

            // Command table. 256 bytes * 32 command headers is 8kb per port
            // Each command has 8 PRDTs, each 256 bytes
            hba_command_hdr_t* hdr = (hba_command_hdr_t*)(hba_mem->ports[port].clb);
            for (uint8_t i = 0; i < AHCI_MAX_CMDS; i++) {
                hdr[i].prdtl = AHCI_PRDT_SIZE;
                hdr[i].ctba = AHCI_START + (40 << 10) + (port << 13) + (i << 8);
            }
 
            // Spin until no commands are running, and issue a start command
            while (hba_mem->ports[port].cmd & 0x8000) {}
            hba_mem->ports[port].cmd |= 0x10;
            hba_mem->ports[port].cmd |= 0x1;
        }
    }

    // Find and setup devices on the SATA bus
    for (uint8_t port = 0; port < AHCI_MAX_PORTS; port++) {
        if (hba_mem->pi & (0x1 << port)) {
            // Assume disk is ATA. Idgaf about anything else
            // Reset the port
            uint32_t spin = 0;
            
            // Clear ST and spin on CR
            hba_mem->ports[port].cmd &= ~(0x1);
            while ((hba_mem->ports[port].cmd & 0x8000) && (spin < 1000000)) {
                spin++;
            }
            spin = 0;

            // Set SCTL.DET to 0x1
            hba_mem->ports[port].sctl &= ((~0x0) << 4);
            hba_mem->ports[port].sctl |= 0x1;
            while (spin <= 1000000) {
                spin++;
            }

            // Set the first 4 bits of SCTL to 0x0
            hba_mem->ports[port].sctl &= ((~0x0) << 4);

            // Set SCTL.DET back to 0x0
            // while ((hba_mem->ports[port].ssts & 0xF) != 0x3) { }
            // NOTE - For some reason, this hangs forever...

            hba_mem->ports[port].serr = ~0x0;

            // Disable slumber and partial state
            hba_mem->ports[port].sctl = 0x301;
            timer_wait(20);
            hba_mem->ports[port].sctl = 0x300;
            timer_wait(20);
            if (hba_mem->cap & (1 << 27)) {
                hba_mem->ports[port].cmd |= ((1 << 1) | (1 << 10) | (0xF << 28));
            }
            timer_wait(20);
            hba_mem->ports[port].serr = 0xFFFFFFFF;
            hba_mem->ports[port].is = 0xFFFFFFFF;

            // Enable command list processing
            hba_mem->ports[port].cmd |= 0x1;

            // Enable interrupts on the port
            hba_mem->ports[port].ie = 0x8;
        }
    }
    
    // Read the cababilities register
    hba_capabilities = (hba_capabilities_t*)pci_bar_table->bar5_addr; // hba_mem->cap is at 0x0 offset

    printf("[AHCI] Initialized.\n");
    return EXIT_SUCCESS;
}

int ahci_read_internal(int port, uintptr_t buffer, uint64_t lba_start, uint32_t sector_count) {
    //printf("[AHCI] Reading %d Sectors on port %d (LBA Start: %d)\n", sector_count, port, lba_start);

    // Clear pending interrupts
    hba_mem->ports[port].is = ~0x0;

    // Check that we are reading from a valid device by checking it's signature
    // TODO - Signature not right, but for some reason is still working
    //printf("Signature: 0x%x\n", hba_mem->ports[port].sig);
    //if (hba_mem->ports[port].sig != 0x00000101) {
        //printf("[AHCI] Device on port %d is not an AHCI device.\n", port);
        //return -EINVAL;
    //}

    uint32_t slots = (hba_mem->ports[port].sact | hba_mem->ports[port].ci);
    int slots_num = hba_capabilities->ncs;
    int slot = 0;
    for (size_t i = 0; i < slots_num; i++) {
        if ((slots & 1) == 0) {
            //printf("Found slot: %d\n", i);
            slot = i;
            break;
        }
        slots >>= 1;
    }

    hba_command_hdr_t* hdr = (hba_command_hdr_t*)(hba_mem->ports[port].clb);
    hdr[slot].cfl = (sizeof(FIS_REG_H2D_t) >> 2); // CFL is the command FIS length in double words, Divide by 4 to get the length we need

    // Set to 0 if reading, else 1
    hdr[slot].prdtl = (uint16_t)(((sector_count - 1) >> 4) + 1);
    if (hdr[slot].prdtl > AHCI_PRDT_SIZE) {
        printf("[AHCI] Max read/write in a single command is 65536 bytes (LBA48).\n");
        return -EINVAL;
    }

    // Get the command table
    hba_command_table_t* tbl = (hba_command_table_t*)hdr[slot].ctba;
    memset(tbl, 0x00, sizeof(hba_command_table_t));

    // Setup the PRDTs. 4MiB per PRDT
    uint16_t i = 0;
    uint32_t sector_count_prdt = sector_count;
    for (i; i < (hdr[slot].prdtl - 1); i++) {
        tbl->prdt_entry[i].dba = buffer - 0xFFFF800000000000; // NOTE - Removing the HHDT offset, PRDT requires physical address
        tbl->prdt_entry[i].dbau = 0; // TODO - Allow 64 bit physical addresses
        tbl->prdt_entry[i].dbc = (4 << 20) - 1;
        tbl->prdt_entry[i].i = 1;
        buffer += (4 << 20) >> 2;
        sector_count_prdt -= 16;
    }
    // Last Entry
    tbl->prdt_entry[i].dba = buffer - 0xFFFF800000000000; // TODO - Same applies here
    tbl->prdt_entry[i].dbau = 0; // TODO - Same applies here
    tbl->prdt_entry[i].dbc = (sector_count_prdt << 9) - 1; 
    tbl->prdt_entry[i].i = 1;

    // Split the uint64_t lba_start into 2 uint32's lba_lo and lba_hi
    uint32_t lba_lo = (uint32_t)(lba_start & 0xFFFFFFFF);
    uint32_t lba_hi = (uint32_t)(lba_start >> 32);

    // Setup the command
    FIS_REG_H2D_t* fis = (FIS_REG_H2D_t*)(&tbl->cfis);

    fis->fis_type = FIS_TYPE_REG_H2D;
    fis->c = 1;
    fis->command = ATA_CMD_READ_DMA_EX;

    fis->lba0 = (uint8_t)lba_lo;
    fis->lba1 = (uint8_t)(lba_lo >> 8);
    fis->lba2 = (uint8_t)(lba_lo >> 16);
    fis->device = 1 << 6; // Set LBA mode

    fis->lba3 = (uint8_t)(lba_lo >> 24);
    fis->lba4 = (uint8_t)lba_hi;
    fis->lba5 = (uint8_t)(lba_hi >> 8);

    fis->countl = sector_count & 0xFF;
    fis->counth = (sector_count >> 8) & 0xFF;

    // Issue command
    uint32_t spin = 0;
    while ((hba_mem->ports[port].tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000) {
        spin++;
    }
    if (spin >= 1000000) {
        printf("[AHCI] Drive was never ready to accept commands.!\n");
        return -EIO;
    }
    hba_mem->ports[port].ci = (1 << slot);
    
    spin = 0;
    while ((hba_mem->ports[port].ci & (1 << slot)) && spin < 1000000) {
        if (hba_mem->ports[port].is & HBA_PxIS_TFES) {
            //printf("[AHCI] Transfer error (IS: 0x%x)\n", hba_mem->ports[port].is);
            // TODO All reports as a transfer error, but its working fine...
            hba_mem->ports[port].is = ~0; // Clear interrupts
            return -EIO;
        }
        spin++;
    }
    
    // Check SERR, if not 0x0, something went wrong
    if (hba_mem->ports[port].serr != 0x00) {
        printf("[AHCI] An error was returned from the AHCI controller. (SERR: 0x%lx)\n", hba_mem->ports[port].serr);
        return -EIO;
    }

    // Clear the interrupt
    hba_mem->ports[port].is = 0x1;

    // Verify data was transferred
    if (hdr->prdbc != sector_count * 512) {
        printf("[AHCI] Not all data was transferred successfully.\n");
        return -EIO;
    }
}

// Main AHCI Read function
// Performs reads on the ahci_read_internal function, ensuring correct function over 64kb
// TODO Pass through error
int ahci_read(int port, uintptr_t buffer, uint64_t lba_start, uint32_t sector_count) {
    printf("[AHCI] Reading %ld sectors on port %d.\n", sector_count, port);
    
    if (sector_count > 128) {
        // Split read into 128 sector chunks
        int remainder = sector_count % 128;
        int full_reads = (sector_count - remainder) / 128;
        printf("%d full reads with %d sectors left.\n", full_reads, remainder);

        uintptr_t curr_buffer = buffer;
        uint64_t curr_lba = lba_start;
        for (size_t i = 0; i < full_reads; i++) {
            ahci_read_internal(port, curr_buffer, curr_lba, 128);
            curr_buffer += 65536;
            curr_lba += 128;
        }
        ahci_read_internal(port, curr_buffer, curr_lba, remainder);
    } else {
        // Size is under 64kb, read normally
        ahci_read_internal(port, buffer, lba_start, sector_count);
    }
}

__attribute__((interrupt))
void ahci_interrupt(void* frame) {
    printf("AHCI interrupt received\n");
}
