#ifndef _ELF_H
#define _ELF_H

// Info from https://github.com/bminor/binutils-gdb/blob/master/include/elf/common.h
#define SECTION_TBL_TYPE_NULL       0
#define SECTION_TBL_TYPE_PROGBITS   1
#define SECTION_TBL_TYPE_SYMTAB     2
#define SECTION_TBL_TYPE_STRTAB     3   // String Table (.text / .rodata)
#define SECTION_TBL_TYPE_NOTE       7

typedef struct {
    uint8_t magic[4];                       // Should be 0x7F 'E' 'L' 'F'
    uint8_t sub_arch;                       // 32bit / 64bit
    uint8_t endianness;                     // Little / Big Endian
    uint8_t elf_hdr_ver;                    // As per the ELF specification, this MUST be 0x1
    uint8_t abi;                            // ELF ABI. 0x0 -> SysV ABI
    uint8_t padding1[8];
    uint16_t type;                          // Binary Type
    uint16_t inst_set;                      // Instruction set
    uint32_t elf_ver;                       // ELF version (NOT the ELF header version)
    uint64_t program_entry_offset;          // Program entry offset
    uint64_t program_hdr_table_offset;
    uint64_t section_hdr_table_offset;
    uint32_t flags;
    uint16_t elf_hdr_size;
    uint16_t program_hdr_table_entry_size;              //
    uint16_t program_hdr_table_entry_nbr;               //
    uint16_t section_hdr_table_entry_size;              //
    uint16_t section_hdr_table_entry_nbr;       //
    uint16_t section_index_shstrtab_table;      // Index of the shstrtab table
} elf_header_t; // ELF Header (64bit ONLY)

typedef struct {
    uint32_t section_name;          // Offset into shstrtab?
    uint32_t section_type;          //
    uint64_t section_flags;         //
    uint64_t section_vaddr_start;   //
    uint64_t section_data_offset;   //
    uint64_t section_size;
    uint32_t section_link;
    uint32_t section_info;
    uint64_t section_alignment;
    uint64_t section_entry_size;
} elf_section_table_t; // ELF Section Table (64 bytes, 0x40, 64bit ONLY)

typedef struct {
    uint32_t segment_type;
    uint32_t flags;
    uint64_t segment_data_offset;
    uint64_t segment_vaddr_start;
    uint64_t segment_paddr;
    uint64_t segment_file_size;
    uint64_t segment_mem_size;
    uint64_t section_alignment;
} elf_program_table_t; // ELF Program Table (56 bytes, 0x38, 64bit ONLY)

void elf_prepare(uintptr_t addr);
int elf_header_parse(uintptr_t addr);
void elf_parse_section_header_table(uintptr_t elf_addr, uintptr_t section_table_addr);
void elf_parse_program_header_table(uintptr_t addr);

int test_strcmp(const char *s1, const char *s2);

#endif
