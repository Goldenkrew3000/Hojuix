/*
// Hojuix NVMe Driver
// 2025-05-16
// Loosely based off of NSG650's Polaris NVMe Driver
// ??? License - not sure TODO
*/

// NOTE: This driver explicitly only works with NVMe drives with their partitions on namespace 1 (Which is every drive I have tested).
// NOTE: This driver ONLY works with a system page size of 4096 bytes and a drive LBA size of 512 bytes (which is extremely common)
// NOTE: This driver reads into CONTIGUOUS PHYSICAL MEMORY

#include <kern/kprintf.h>
#include <drivers/nvme.h>
#include <memory/pmmgr.h>
#include <memory/vmmgr.h>
#include <drivers/pci.h>
#include <arch/amd64/asm_functions.h>

// Globals
uint32_t spin = 0;
int rc = 0;
uint64_t max_prps = 0;
uint64_t nvme_base_addr = 0;
volatile uint32_t* nvme_mmio;
struct nvme_queue admin_queue;
struct nvme_queue ns_queue;
uintptr_t ns1_io_queue_regpgs = 0;

uintptr_t glob_test = 0;

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
    glob_test = admin_completion_addr;
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
    admin_queue.phys_regpgs = 0; // Should be null?

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
    printf("%x %x %x %x %x\n", identify_cns2_ptr[0], identify_cns2_ptr[1], identify_cns2_ptr[2], identify_cns2_ptr[3], identify_cns2_ptr[4]);
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

    // Calculate some needed stuff
    size_t shift = 12 + CAP_MIN_PAGE_SIZE(capabilities);
    size_t max_trans_shift = 0;
    if (identify_ns0->mdts) {
        max_trans_shift = shift + identify_ns0->mdts;
    } else {
        max_trans_shift = 20;
    }
    uint64_t formatted_lba = identify_ns1->flbas & 0x0f;
    uint64_t lba_shift = identify_ns1->lbaf[formatted_lba].lba_data_size;
    uint64_t max_lba_size = 1 << (max_trans_shift - lba_shift);
    max_prps = (max_lba_size * (1 << lba_shift)) / PAGE_SIZE;
    printf("[NVMe] Max PRPs: %lld\n", max_prps);

    // Create and assign IO SQ/CQ Queues to Namespace 1
    int queue_id = 1; // Namespace ID
    uintptr_t ns1_io_queue_submit_addr = pmmgr_kmalloc_contiguous(16) + 0xFFFF800000000000; // TODO Not sure of the size
    uintptr_t ns1_io_queue_completion_addr = pmmgr_kmalloc_contiguous(16) + 0xFFFF800000000000; // TODO Not sure of the size
    ns1_io_queue_regpgs = pmmgr_kmalloc_contiguous(16); // not sure
    i386_memset((uint8_t*)ns1_io_queue_submit_addr, 0x00, 4096 * 16); // TODO Not sure of the size
    i386_memset((uint8_t*)ns1_io_queue_completion_addr, 0x00, 4096 * 16); // TODO Not sure of the size
    i386_memset((uint8_t*)(ns1_io_queue_regpgs + 0xFFFF800000000000), 0x00, 4096 * 16); // not sure
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
	//ns_queue.phys_regpgs = (uint64_t*)ns1_io_queue_regpgs;
    //ns_queue.phys_regpgs = (uint64_t)ns1_io_queue_regpgs;

    
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
    /*while (true) {
        status = queue->completion[queue->cq_head].status;
        if ((status && 0x01) == phase) {
            break;
        }
    }*/

    head++;
    if (head == queue->elements) {
		head = 0;
		queue->cq_phase = !queue->cq_phase;
	}

    nvme_mmio_out32(queue->complete_db, head);
    queue->cq_head = head;
	return status;
}

struct nvme_prp_list {
    uint64_t entries[PAGE_SIZE / sizeof(uint64_t)]; // 512 entries per page
};

// Main NVMe read function. Buffer is a virtual address
int nvme_read(uintptr_t buffer, uint32_t start_lba, uint32_t lba_count) {
    if (lba_count <= 8) {
        // Single page read
        struct nvme_cmd read_cmd = {0};
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
        read_cmd.rw.prp1 = (uint64_t)(buffer - 0xFFFF800000000000);
        nvme_submit_wait_cmd(&ns_queue, read_cmd);
    } else if (lba_count <= 16) {
        // 2 page read (NOTE: NOT TESTED)
        struct nvme_cmd read_cmd = {0};
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
        read_cmd.rw.prp1 = (uint64_t)(buffer - 0xFFFF800000000000);
        read_cmd.rw.prp2 = (uint64_t)(buffer - 0xFFFF800000000000) + PAGE_SIZE;
        nvme_submit_wait_cmd(&ns_queue, read_cmd);
    } else {
        // First use of PRPs, im fucked here aren't I
        size_t prp_count = ((lba_count - 1) * 512) / PAGE_SIZE;
        //printf("PRP count: %ld\n", prp_count);
        if (prp_count > max_prps) {
            printf("[NVMe] Read is over maximum PRP count.\n");
            return EXIT_FAILURE;
        } else {
            //printf("Performing PRP read.\n");
        }

        struct nvme_cmd read_cmd = {0};
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
        read_cmd.rw.prp1 = (uint64_t)buffer - 0xFFFF800000000000;
        
        uint64_t* regpgs_virt = (uint64_t*)(ns1_io_queue_regpgs + 0xFFFF800000000000);
        for (size_t i = 0; i < prp_count; i++) {
            regpgs_virt[i] = (buffer - 0xFFFF800000000000) + (i * PAGE_SIZE) + PAGE_SIZE;
        }
        read_cmd.rw.prp2 = (uint64_t)ns1_io_queue_regpgs; // TODO zero page before modifying it (prp page)
        
        nvme_submit_wait_cmd(&ns_queue, read_cmd);
    }

    
    
    return EXIT_SUCCESS;
    // TODO Return code
}

void nvme_read_smart() {
    uintptr_t buf = pmmgr_kcalloc(1);
    struct nvme_cmd smart_cmd = {0};
    smart_cmd.log.opcode = 0x02; // Admin Log Page Opcode
    smart_cmd.log.nsid = 0xFFFFFFFF; // Standard for SMART
    smart_cmd.log.prp1 = (uint64_t)buf;
    smart_cmd.log.cdw10 = (0x02) | ((127) << 16);
    smart_cmd.log.cdw12 = 0;
    rc = nvme_submit_wait_cmd(&admin_queue, smart_cmd);
    if (rc != 0) {
        printf("[NVMe] Fatal error received on SMART data cmd.\n");
        return;
    }

    nvme_admin_smart_log_page_t* smart_log = (nvme_admin_smart_log_page_t*)(buf + 0xFFFF800000000000);
    // TODO figure out why there are extra zeros here...
    printf("CW: %d\n", smart_log->cw);
    printf("CTEMP (Kv): %d\n", smart_log->ctemp);
    printf("AVSP: %d\n", smart_log->avsp);
    printf("AVSPT: %d\n", smart_log->avspt);
    printf("PUSED: %d%%\n", smart_log->pused);
    printf("EGCWS: %d\n", smart_log->egcws);
    printf("DUR: %.16llx%.16llx\n", smart_log->dur_hi, smart_log->dur_lo);
    printf("DUW: %.16llx%.16llx\n", smart_log->duw_hi, smart_log->duw_lo);
    printf("HRC: %.16llx%.16llx\n", smart_log->hrc_hi, smart_log->hrc_lo);
    printf("HWC: %.16llx%.16llx\n", smart_log->hwc_hi, smart_log->hwc_lo);
    printf("CBT: %.16llx%.16llx\n", smart_log->cbt_hi, smart_log->cbt_lo);
    printf("PWRC: %.16llx%.16llx\n", smart_log->pwrc_hi, smart_log->pwrc_lo);
    printf("POH: %.16llx%.16llx\n", smart_log->poh_hi, smart_log->poh_lo);
    printf("UPL: %.16llx%.16llx\n", smart_log->upl_hi, smart_log->upl_lo);
    printf("MDIE: %.16llx%.16llx\n", smart_log->mdie_hi, smart_log->mdie_lo);
    printf("NEILE: %.16llx%.16llx\n", smart_log->neile_hi, smart_log->neile_lo);
    printf("WCTT: %ld\n", smart_log->wctt);
    printf("CCTT: %ld\n", smart_log->cctt);
    printf("TSEN1: %ld\n", smart_log->tsen1);
    printf("TSEN2: %ld\n", smart_log->tsen2);
}

/*
// MMIO Functions (TODO Unify across the kernel)
*/
uint32_t nvme_mmio_in32(uint64_t base) {
    return *(volatile uint32_t *) base;
}

void nvme_mmio_out32(uint64_t base, uint32_t value) {
    *(volatile uint32_t *) base = value;
}