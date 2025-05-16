#ifndef _NVME_H
#define _NVME_H
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Next 3 structs are from NSG650's Polaris NVME Driver
struct nvme_cmd {
    union {
        struct {
            uint8_t opcode;
            uint8_t flags;
            uint16_t cid;
            uint32_t nsid;
            uint32_t cdw1[2];
            uint64_t metadata;
            uint64_t prp1;
            uint64_t prp2;
            uint32_t cdw2[6];
        } common; // generic command
        struct {
            uint8_t opcode;
            uint8_t flags;
            uint16_t cid;
            uint32_t nsid;
            uint64_t unused;
            uint64_t metadata;
            uint64_t prp1;
            uint64_t prp2;
            uint64_t slba;
            uint16_t len;
            uint16_t control;
            uint32_t dsmgmt;
            uint32_t ref;
            uint16_t apptag;
            uint16_t appmask;
        } rw; // read or write
        struct {
            uint8_t opcode;
            uint8_t flags;
            uint16_t cid;
            uint32_t nsid;
            uint64_t unused1;
            uint64_t unused2;
            uint64_t prp1;
            uint64_t prp2;
            uint32_t cns;
            uint32_t unused3[5];
        } identify; // identify
        struct {
            uint8_t opcode;
            uint8_t flags;
            uint16_t cid;
            uint32_t nsid;
            uint64_t unused1;
            uint64_t unused2;
            uint64_t prp1;
            uint64_t prp2;
            uint32_t fid;
            uint32_t dword;
            uint64_t unused[2];
        } features;
        struct {
            uint8_t opcode;
            uint8_t flags;
            uint16_t cid;
            uint32_t unused1[5];
            uint64_t prp1;
            uint64_t unused2;
            uint16_t cqid;
            uint16_t size;
            uint16_t cqflags;
            uint16_t irqvec;
            uint64_t unused3[2];
        } createcompq;
        struct {
            uint8_t opcode;
            uint8_t flags;
            uint16_t cid;
            uint32_t unused1[5];
            uint64_t prp1;
            uint64_t unused2;
            uint16_t sqid;
            uint16_t size;
            uint16_t sqflags;
            uint16_t cqid;
            uint64_t unused3[2];
        } createsubq;
        struct {
            uint8_t opcode;
            uint8_t flags;
            uint16_t cid;
            uint32_t unused1[9];
            uint16_t qid;
            uint16_t unused2;
            uint32_t unused3[5];
        } deleteq;
        struct {
            uint8_t opcode;
            uint8_t flags;
            uint16_t cid;
            uint32_t unused1[9];
            uint16_t sqid;
            uint16_t cqid;
            uint32_t unused2[5];
        } abort;
    };
};

struct nvme_cmd_comp {
    uint32_t result;
    uint32_t reserved;
    uint16_t sq_head;
    uint16_t sq_id;
    uint16_t cmd_id;
    uint16_t status;
};

struct nvme_queue {
    volatile struct nvme_cmd *submit;
    volatile struct nvme_cmd_comp *completion;
    uint64_t submit_db;
    uint64_t complete_db;
    uint16_t elements; // elements in queue
    uint16_t cq_vec;
    uint16_t sq_head;
    uint16_t sq_tail;
    uint16_t cq_head;
    uint8_t cq_phase;
    uint16_t queue_id;	   // queue id
    uint32_t cmd_id;	   // command id
    uint64_t *phys_regpgs; // pointer to the PRPs
};

// Rest of the structs are made by me
typedef struct { // NVM Express Base Specification Rev 1.2 Page 296 (5.1.13.2.1)
    uint16_t vid;           // PCI Vendor ID
    uint16_t ssvid;         // PCI Subsystem Vendor ID
    uint8_t sn[20];         // Serial Number (ASCII String)
    uint8_t mn[40];         // Model Number (ASCII String)
    uint8_t fr[8];          // Firmware Revision (ASCII String)
    uint8_t rab;            // Recommended Arbitration Burst
    uint8_t ieee[3];        // IEEE OUI (Organization Unique Identifier) Identifier
    uint8_t cmic;           // Controller Multi-Path I/O and Namespace Sharing Capabilities
    uint8_t mdts;           // Maximum Data Transfer Size
    uint16_t cntlid;        // Controller ID
    uint32_t ver;           // Version
    uint32_t uu1[43]; // TO CHANGE
    uint16_t oacs;
    uint8_t acl;
    uint8_t aerl;
    uint8_t fw;
    uint8_t lpa;
    uint8_t elpe;
    uint8_t npss;
    uint8_t avscc;
    uint8_t apsta;
    uint16_t wctemp;
    uint16_t cctemp;
    uint16_t uu2[121]; // TO CHANGE
    uint8_t sqes;
    uint8_t cqrs;
    uint16_t uu3; // to change
    uint32_t nn;
} __attribute__((packed)) nvme_identify_ns0_t; // NVMe Identify Namespace 0 (CNS 1)

typedef struct { // NVM Express NVM Command Set Specification Rev 1.1 Page 80 (Figure 114)
    uint64_t nsze;          // Namespace Size (in LBAs)
    uint64_t ncap;          // Namespace Capacity (in LBAs)
    uint64_t nuse;          // Namespace Utilization
} __attribute__((packed)) nvme_identify_nsx_cns0_t; // NVMe Identify NSx CNS 0

// NVME -
#define CAP_MAX_ENTRIES(cap) ((cap) & 0xffff)
#define CAP_DOORBELL_STRIDE(cap) (((cap) >> 32) & 0xf)
#define CAP_COMMAND_SET(cap) (((cap) >> 37) & 0xff)

// NVME -
#define CC_COMMANDSET_NVM (0 << 4)
#define CC_ARBITRATION_ROUNDROBIN (0 << 11)
#define CC_SHUTDOWN_NOTIFICATIONS_NONE (0 << 14)
#define CC_IO_SUBMISSION_QUEUE_SIZE (6 << 16)
#define CC_IO_COMPLETION_QUEUE_SIZE (4 << 20)
#define CC_ENABLE (1)

// NVMe Registers
#define NVME_REG_CAP      0x0000  // Capabilities
#define NVME_REG_VS       0x0008  // Version
#define NVME_REG_CC       0x0014  // Controller Configuration
#define NVME_REG_CSTS     0x001C  // Controller Status
#define NVME_REG_AQA      0x0024  // Admin Queue Attributes
#define NVME_REG_ASQ      0x0028  // Admin Submission Queue Base
#define NVME_REG_ACQ      0x0030  // Admin Completion Queue Base
#define NVME_REG_SQ0TDBL  0x1000  // Admin SQ Tail Doorbell
#define NVME_REG_CQ0HDB 0x1004  // Completion Queue 0 Head Doorbell (in BAR1)

// NVME Opcodes
#define NVME_OPCODE_FLUSH               0x00
#define NVME_OPCODE_ADMIN_CREATE_SQ     0x01
#define NVME_OPCODE_WRITE               0x01
#define NVME_OPCODE_READ                0x02
#define NVME_OPCODE_ADMIN_DELETE_CQ     0x04
#define NVME_OPCODE_ADMIN_CREATE_CQ     0x05
#define NVME_OPCODE_ADMIN_IDENTIFY      0x06
#define NVME_OPCODE_ADMIN_ABORT         0x08
#define NVME_OPCODE_ADMIN_SET_FT        0x09
#define NVME_OPCODE_ADMIN_GET_FT        0x0A

#define NVME_SPIN_TIMEOUT 2500000 // 250만 사이클

int nvme_init(uintptr_t bar_tbl_addr);
void nvme_wait_completion(uint16_t expected_cid);
void nvme_submit_cmd(struct nvme_queue* queue, struct nvme_cmd cmd);
int nvme_submit_wait_cmd(struct nvme_queue* queue, struct nvme_cmd cmd);


int nvme_read(uintptr_t buffer, uint32_t start_lba, uint32_t lba_count);
uint32_t nvme_mmio_in32(uint64_t base);
void nvme_mmio_out32(uint64_t base, uint32_t value);

#endif

