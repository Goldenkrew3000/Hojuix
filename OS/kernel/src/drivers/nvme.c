/*
// Hojuix NVMe Driver
// 2025-05-16
// Loosely based off of NSG650's Polaris NVMe Driver
// ??? License - not sure TODO
*/

// NOTE: This driver explicitly only works with NVMe drives with their partitions on namespace 1 (Which is every drive I have tested).

#include <stdio.h>
#include <drivers/nvme.h>
#include <memory/pmmgr.h>
#include <memory/vmmgr.h>
#include <drivers/pci.h>
#include <i386/asm_functions.h>

// Globals
uint32_t spin = 0;
int rc = 0;
uint64_t nvme_base_addr = 0;
volatile uint32_t* nvme_mmio;
struct nvme_queue admin_queue;
struct nvme_queue ns_queue;

int nvme_init(uintptr_t bar_tbl_addr) {
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

    // Forge NVMe Address and map it
    // TODO Check if address is 32 bit accessable, and go from there
    nvme_base_addr = (uint64_t)(((uint64_t)pci_bar_table->bar1_full << 32) | (pci_bar_table->bar0_full & 0xFFFFFFF0));
    vmmgr_mmio_map_uncache(nvme_base_addr, nvme_base_addr, 4); // TODO Dont need 4 pages
    nvme_mmio = (uint32_t*)nvme_base_addr; // Assign MMIO pointer

    /*
    // NVMe Controller Setup
    */
    // Reset the NVME controller
    printf("[NVMe] Resetting controller...");
    uint32_t cc = *(volatile uint32_t*)(nvme_base_addr + NVME_REG_CC);
    cc &= ~(1 << 0); // Clear the EN bit
    *(volatile uint32_t*)(nvme_base_addr + NVME_REG_CC) = cc;
    
    // Wait for controller to be ready (CSTS.RDY == 0)
    spin = 0;
    while (*(volatile uint32_t*)(nvme_base_addr + NVME_REG_CSTS) & (1 << 0)) {
        spin++;
        if (spin >= NVME_SPIN_TIMEOUT) {
            printf(" FAIL.\n");
            return -EBUSY; // Controller was never ready after reset
        }
    }
    printf(" OK.\n");

    // Get NVMe Version
    uint32_t nvme_vs = *(volatile uint32_t*)(nvme_base_addr + NVME_REG_VS);
    uint8_t nvme_vs_major = (nvme_vs >> 16) & 0xFF; // hmmm read spec on the size of this TODO
    uint8_t nvme_vs_minor = (nvme_vs >> 8) & 0xFF;
    printf("[NVMe] Protocol Version: %d.%d\n", nvme_vs_major, nvme_vs_minor);

    // Read capabilities
    uint64_t capabilities = *(volatile uint64_t*)(nvme_base_addr + NVME_REG_CAP);
    uint8_t stride = CAP_DOORBELL_STRIDE(capabilities);
    uint16_t queue_slots = CAP_MAX_ENTRIES(capabilities);

    // Create admin queue
    int admin_queue_id = 0; // First queue is the admin queue
    uintptr_t admin_submit_addr = pmmgr_kmalloc_contiguous(4) + 0xFFFF800000000000; // TODO Not sure of the size
    uintptr_t admin_completion_addr = pmmgr_kmalloc_contiguous(4) + 0xFFFF800000000000; // TODO Not sure of the size
    memset((uint8_t*)admin_submit_addr, 0x00, 4096 * 4); // TODO Not sure of the size
    memset((uint8_t*)admin_completion_addr, 0x00, 4096 * 4); // TODO Not sure of the size
    admin_queue.submit = (struct nvme_cmd*)admin_submit_addr;
    admin_queue.submit_db = (nvme_base_addr + PAGE_SIZE + (2 * admin_queue_id * (4 << stride)));
    admin_queue.sq_head = 0;
    admin_queue.sq_tail = 0;
    admin_queue.completion = (struct nvme_cmd_comp*)admin_completion_addr;
    admin_queue.complete_db = (nvme_base_addr + PAGE_SIZE + ((2 * admin_queue_id + 1) * (4 << stride)));
    admin_queue.cq_vec = 0;
    admin_queue.cq_head = 0;
    admin_queue.cq_phase = 1;
    admin_queue.elements = queue_slots;
    admin_queue.queue_id = admin_queue_id;
    admin_queue.cmd_id = 0;
    admin_queue.phys_regpgs = NULL;

    // Assign admin queue
    uint32_t admin_queue_attrs = queue_slots - 1;
    admin_queue_attrs |= admin_queue_attrs << 16;
	admin_queue_attrs |= admin_queue_attrs << 16;
    nvme_mmio[NVME_REG_AQA/4] = (0xF << 0) | (0xF << 16);  // ASQ/ACQ size = 16 entries TODO fix
    nvme_mmio[NVME_REG_ASQ/4] = (uint32_t)(admin_queue.submit - 0xFFFF800000000000);
    nvme_mmio[NVME_REG_ACQ/4] = (uint32_t)(admin_queue.completion - 0xFFFF800000000000);

    // Enable controller
    printf("[NVMe] Enabling controller...");
    uint32_t cc1 = nvme_mmio[NVME_REG_CC/4];
    cc1 = CC_COMMANDSET_NVM | CC_ARBITRATION_ROUNDROBIN | CC_SHUTDOWN_NOTIFICATIONS_NONE | CC_IO_SUBMISSION_QUEUE_SIZE | CC_IO_COMPLETION_QUEUE_SIZE | CC_ENABLE;
    nvme_mmio[NVME_REG_CC/4] = cc1;
    spin = 0;
    while (!(nvme_mmio[NVME_REG_CSTS/4] & 0x1)) {
        spin++;
        if (spin >= NVME_SPIN_TIMEOUT) {
            printf(" FAIL.\n");
            return -EBUSY; // Controller was never ready
        }
    }
    printf(" OK.\n");

    // Run IDENTIFY on Namespace 0 (This fetches the drive's main details)
    uintptr_t identify_ns0_buffer = pmmgr_kmalloc_contiguous(1);
    memset((uint8_t*)(identify_ns0_buffer + 0xFFFF800000000000), 0x00, 4096);

    struct nvme_cmd identify_ns0_cmd = {0};
    identify_ns0_cmd.identify.opcode = NVME_OPCODE_ADMIN_IDENTIFY;
	identify_ns0_cmd.identify.nsid = 0;
	identify_ns0_cmd.identify.cns = 1;
    identify_ns0_cmd.identify.prp1 = (uint64_t)identify_ns0_buffer;
    identify_ns0_cmd.identify.prp2 = 0;
    rc = nvme_submit_wait_cmd(&admin_queue, identify_ns0_cmd);
    if (rc != 0) {
        printf("[NVMe] Fatal error received on IDENTIFY (NS0 stage).\n");
        return -EIO;
    }

    nvme_identify_ns0_t* identify_ns0 = (nvme_identify_ns0_t*)(identify_ns0_buffer + 0xFFFF800000000000);
    printf("[NVMe] NVMe Model: %.40s\n", identify_ns0->mn);
    printf("[NVMe] NVMe Serial Number: %.20s\n", identify_ns0->sn);
    printf("[NVMe] NVMe Firmware Revision: %.8s\n", identify_ns0->fr);

    // Run IDENTIFY on CNS 2 (This fetches the drive's active namespaces)
    uintptr_t identify_cns2_buffer = pmmgr_kmalloc_contiguous(1);
    memset((uint8_t*)(identify_cns2_buffer + 0xFFFF800000000000), 0x00, 4096);

    struct nvme_cmd identify_cns2_cmd = {0};
    identify_cns2_cmd.identify.opcode = NVME_OPCODE_ADMIN_IDENTIFY;
	identify_cns2_cmd.identify.cns = 2;
    identify_cns2_cmd.identify.prp1 = (uint64_t)identify_cns2_buffer;
    rc = nvme_submit_wait_cmd(&admin_queue, identify_cns2_cmd);
    if (rc != 0) {
        printf("[NVMe] Fatal error received on IDENTIFY (CNS2 stage).\n");
        return -EIO;
    }

    uint8_t* identify_cns2_ptr = (uint8_t*)(identify_cns2_buffer + 0xFFFF800000000000);
    bool foundNamespace1 = false;
    for (size_t i = 0; i < identify_ns0->nn; i++) {
        if (identify_cns2_ptr[i]) {
            if (identify_cns2_ptr[i] != 0x1) {
                printf("[NVMe] WARNING: Namespace other than 1 identified.\n");
            } else {
                printf("[NVMe] Identified namespace 1.\n");
                foundNamespace1 = true;
            }
        }
    }
    if (!foundNamespace1) {
        printf("[NVMe] Could not find namespace 1.\n");
        return -EINVAL;
    }

    // Run identify on namespace 1 (This gets the drive's actual specifications)
    uintptr_t identify_ns1_buffer = pmmgr_kmalloc_contiguous(1);
    memset((uint8_t*)(identify_ns1_buffer + 0xFFFF800000000000), 0x00, 4096);

    struct nvme_cmd identify_ns1_cmd = {0};
    identify_ns1_cmd.identify.opcode = NVME_OPCODE_ADMIN_IDENTIFY;
    identify_ns1_cmd.identify.nsid = 1;
	identify_ns1_cmd.identify.cns = 0;
    identify_ns1_cmd.identify.prp1 = (uint64_t)identify_ns1_buffer;
    rc = nvme_submit_wait_cmd(&admin_queue, identify_ns1_cmd);
    if (rc != 0) {
        printf("[NVMe] Fatal error received on IDENTIFY (NS1 stage).\n");
        return -EIO;
    }

    nvme_identify_nsx_cns0_t* identify_ns1 = (nvme_identify_nsx_cns0_t*)(identify_ns1_buffer + 0xFFFF800000000000);
    printf("[NVMe] Namespace 1 Size (LBAs/GBs): %lld LBAs / %lld GBs\n", identify_ns1->nsze, ((identify_ns1->nsze * 512) / 1024 / 1024 / 1024));

    // Create and assign IO SQ/CQ Queues to Namespace 1
    int queue_id = 1; // Namespace ID
    uintptr_t ns1_io_queue_submit_addr = pmmgr_kmalloc_contiguous(4) + 0xFFFF800000000000; // TODO Not sure of the size
    uintptr_t ns1_io_queue_completion_addr = pmmgr_kmalloc_contiguous(4) + 0xFFFF800000000000; // TODO Not sure of the size
    printf("ns1 comp: %llx\n", ns1_io_queue_completion_addr);
    memset((uint8_t*)ns1_io_queue_submit_addr, 0x00, 4096 * 4); // TODO Not sure of the size
    memset((uint8_t*)ns1_io_queue_completion_addr, 0x00, 4096 * 4); // TODO Not sure of the size
    ns_queue.submit = (struct nvme_cmd*)ns1_io_queue_submit_addr;
	ns_queue.submit_db = (nvme_base_addr + PAGE_SIZE + (2 * queue_id * (4 << stride)));
	ns_queue.sq_head = 0;
	ns_queue.sq_tail = 0;
	ns_queue.completion = (struct nvme_cmd_comp*)ns1_io_queue_completion_addr;
	ns_queue.complete_db = (nvme_base_addr + PAGE_SIZE + ((2 * queue_id + 1) * (4 << stride)));
	ns_queue.cq_vec = 0;
	ns_queue.cq_head = 0;
	ns_queue.cq_phase = 1;
	ns_queue.elements = queue_slots;
	ns_queue.queue_id = queue_id;
	ns_queue.cmd_id = 0;
	ns_queue.phys_regpgs = (uint64_t*)pmmgr_kmalloc_contiguous(4); // TODO not sure of the size
    
    struct nvme_cmd ns1_io_queue_completion_cmd = {0};
	ns1_io_queue_completion_cmd.createcompq.opcode = NVME_OPCODE_ADMIN_CREATE_CQ;
	ns1_io_queue_completion_cmd.createcompq.prp1 = (uint64_t)(ns1_io_queue_completion_addr - 0xFFFF800000000000);
	ns1_io_queue_completion_cmd.createcompq.cqid = queue_id;
	ns1_io_queue_completion_cmd.createcompq.size = queue_slots - 1;
	ns1_io_queue_completion_cmd.createcompq.cqflags = (1 << 0);
	ns1_io_queue_completion_cmd.createcompq.irqvec = 0;
    rc = nvme_submit_wait_cmd(&admin_queue, ns1_io_queue_completion_cmd);
    if (rc != 0) {
        printf("[NVMe] Fatal error received on IDENTIFY (NS1 Create CQ stage).\n");
        return -EIO;
    }

    struct nvme_cmd ns1_io_queue_submit_cmd = {0};
	ns1_io_queue_submit_cmd.createsubq.opcode = NVME_OPCODE_ADMIN_CREATE_SQ;
	ns1_io_queue_submit_cmd.createsubq.prp1 = (uint64_t)(ns1_io_queue_submit_addr - 0xFFFF800000000000);
	ns1_io_queue_submit_cmd.createsubq.sqid = queue_id;
	ns1_io_queue_submit_cmd.createsubq.cqid = queue_id;
	ns1_io_queue_submit_cmd.createsubq.size = queue_slots - 1;
	ns1_io_queue_submit_cmd.createsubq.sqflags = (1 << 0) | (2 << 1); // queue phys + medium priority
    rc = nvme_submit_wait_cmd(&admin_queue, ns1_io_queue_submit_cmd);
    if (rc != 0) {
        printf("[NVMe] Fatal error received on IDENTIFY (NS1 Create SQ stage).\n");
        return -EIO;
    }

    // NVMe initialization completed successfully
    printf("[NVMe] Initialized.\n");
    return EXIT_SUCCESS;
}

void nvme_submit_cmd(struct nvme_queue* queue, struct nvme_cmd cmd) {
    uint16_t tail = queue->sq_tail;
    queue->submit[tail] = cmd;
    tail++;
    if (tail == queue->elements) {
        tail = 0;
    }
    nvme_mmio_out32(queue->submit_db, tail);
    queue->sq_tail = tail;
}

int nvme_submit_wait_cmd(struct nvme_queue* queue, struct nvme_cmd cmd) {
    uint16_t head = queue->cq_head;
	uint16_t phase = queue->cq_phase;
    cmd.common.cid = queue->cmd_id++;
	nvme_submit_cmd(queue, cmd);
	uint16_t status = 0; // TODO Actually return status

    spin = 0;
    while ((queue->completion[head].status & 0x01) != phase) {
        //status = queue->completion[head].status;
        spin++;
        if (spin >= NVME_SPIN_TIMEOUT) {
            printf("[NVMe] NVMe Command Timeout.\n");
            return -EIO;
        }
    }

    head++;
    if (head == queue->elements) {
		head = 0;
		queue->cq_phase = !queue->cq_phase;
	}

    nvme_mmio_out32(queue->complete_db, head);
    queue->cq_head = head;
	return status;
}

int nvme_read(uintptr_t buffer, uint32_t start_lba, uint32_t lba_count) {
    // PRP-less method for testing
    // 1 PRP allows for a 4096 byte (8 LBAs of read)
    int remainder = lba_count % 8;
    int full_reads = (lba_count - remainder) / 8;
    printf("[NVMe] %d full reads with %d sectors left.\n", full_reads, remainder);
    printf("BFR: %llx\n", buffer);

    uintptr_t curr_buffer = buffer;
    uint64_t curr_lba = start_lba;
    
    struct nvme_cmd read_cmd = {0};
    for (size_t i = 0; i < full_reads; i++) {
        //printf("Read: %llx -- %d\n", curr_buffer, curr_lba);
        read_cmd.rw.opcode = NVME_OPCODE_READ;
        read_cmd.rw.flags = 0;
        read_cmd.rw.nsid = 1;
        read_cmd.rw.control = 0;
        read_cmd.rw.dsmgmt = 0;
        read_cmd.rw.ref = 0;
        read_cmd.rw.apptag = 0;
        read_cmd.rw.appmask = 0;
        read_cmd.rw.metadata = 0;
        read_cmd.rw.slba = curr_lba;
        read_cmd.rw.len = 7;
        read_cmd.rw.prp1 = (uint64_t)(curr_buffer - 0xFFFF800000000000);
        nvme_submit_wait_cmd(&ns_queue, read_cmd);
        curr_buffer += 4096;
        curr_lba += 8;
    }
    /*
    ahci_read_internal(port, curr_buffer, curr_lba, remainder);

    for (size_t i = 0; i < lba_count)
    
	read_cmd.rw.opcode = NVME_OPCODE_READ;
	read_cmd.rw.flags = 0;
	read_cmd.rw.nsid = 1;
	read_cmd.rw.control = 0;
	read_cmd.rw.dsmgmt = 0;
	read_cmd.rw.ref = 0;
	read_cmd.rw.apptag = 0;
	read_cmd.rw.appmask = 0;
	read_cmd.rw.metadata = 0;
	read_cmd.rw.slba = start_lba;
	read_cmd.rw.len = lba_count - 1;
    read_cmd.rw.prp1 = (uint64_t)buffer;
    //cmd5.rw.prp2 = 0;
    nvme_submit_wait_cmd(&ns_queue, read_cmd);*/
    return EXIT_SUCCESS;
    // TODO Return code
}

/*
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
*/


/*
// MMIO Functions (TODO Unify across the kernel)
*/
uint32_t nvme_mmio_in32(uint64_t base) {
    return *(volatile uint32_t *) base;
}

void nvme_mmio_out32(uint64_t base, uint32_t value) {
    *(volatile uint32_t *) base = value;
}