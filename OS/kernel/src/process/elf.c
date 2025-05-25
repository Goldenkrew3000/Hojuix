/*
// ELF Parser v1.0
// Hojuix 2025-04-20
// ELF Specification: http://www.skyfree.org/linux/references/ELF_Format.pdf
// NOTE: Does not follow the ELF Specification naming convention, (maybe) fix later.
*/

#include <stdint.h>
#include <kern/kprintf.h>
#include <process/elf.h>
#include <memory/vmmgr.h>
#include <arch/amd64/asm_functions.h>

uintptr_t program_header_addr;
uintptr_t section_header_addr;
elf_header_t* elf_header;
elf_section_table_t* elf_text_section;
elf_section_table_t* elf_rodata_section;

void elf_prepare(uintptr_t addr) {
    printf("[ELF] Parsing ELF file...\n");

    // Parse the ELF header
    elf_header_parse(addr);

    //elf_parse_program_header_table(program_header_addr);
    elf_parse_section_header_table(addr, section_header_addr);

    // TEST: Copy the .text data to the correct spot
    uintptr_t text_data = addr + elf_text_section->section_data_offset;
    printf("TEXT: %llx\n", text_data);
    printf("[ELF] Virtual address to copy .text to: 0x%lx\n", elf_text_section->section_vaddr_start);
    i386_memcpy((uintptr_t)elf_text_section->section_vaddr_start, text_data, elf_text_section->section_size);

    uintptr_t rodata_data = addr + elf_rodata_section->section_data_offset;
    printf("RODATA: %llx\n", rodata_data);
    printf("[ELF] Virtual address to copy .rodata to: 0x%lx\n", elf_rodata_section->section_vaddr_start);
    i386_memcpy((uintptr_t)elf_rodata_section->section_vaddr_start, rodata_data, elf_rodata_section->section_size);
    printf("done\n");
}

void elf_header_parse(uintptr_t addr) { // TODO - Add a struct table for checks
    elf_header = (elf_header_t*)addr;

    // Verify ELF header
    if (elf_header->magic[0] != 0x7F &&
        elf_header->magic[1] != 0x45 &&
        elf_header->magic[2] != 0x4C &&
        elf_header->magic[3] != 0x46) {
            printf("[ELF] File is not an ELF file.\n");
    } else {
        // found
    }

    // Check ELF sub architecture (32bit or 64bit)
    if (elf_header->sub_arch == 0x0) {
        printf("[ELF] Unknown sub-architecture.\n");
    } else if (elf_header->sub_arch == 0x1) {
        printf("[ELF] 32-bit ELF executables are not supported.\n");
    } else if (elf_header->sub_arch == 0x2) {
        // found
    } else {
        printf("[ELF] Unknown sub-architecture.\n");
    }

    // Check ELF endianness
    if (elf_header->endianness == 0x0) {
        printf("[ELF] Unknown endianness.\n");
    } else if (elf_header->endianness == 0x1) {
        // found
    } else if (elf_header->endianness == 0x2) {
        printf("[ELF] Big endian ELF executables are not supported.\n");
    } else {
        printf("[ELF] Unknown endianness.\n");
    }

    // Check ELF header version
    if (elf_header->elf_hdr_ver != 0x1) {
        printf("[ELF] ELF header version is incorrect.\n");
    } else {
        // found
    }

    // Check ELF ABI for SysV compatibility
    if (elf_header->abi != 0x0) {
        printf("[ELF] ELF ABI does not match System-V.\n");
    } else {
        //found
    }

    // Check ELF type
    if (elf_header->type == 0x0) {
        printf("[ELF] ELF has no type.\n");
    } else if (elf_header->type == 0x1) {
        printf("[ELF] ELF type 'relocatable' is not supported.\n");
    } else if (elf_header->type == 0x2) {
        // found executable
        printf("[ELF] Found ELF executable.\n");
    } else if (elf_header->type == 0x3) {
        printf("[ELF] ELF type 'shared' is not supported.\n");
    } else if (elf_header->type == 0x4) {
        printf("[ELF] ELF type 'core' is not supported.\n");
    } else {
        printf("[ELF] ELF has unknown type.\n");
    }

    // Check ELF instruction set TODO FILL OUT
    if (elf_header->inst_set == 0x0) {
        printf("[ELF] ELF instruction set is unknown.\n");
    } else if (elf_header->inst_set == 0x2) {
        printf("[ELF] ELF instruction set 'sparc' is not supported.\n");
    } else if (elf_header->inst_set == 0x3) {
        printf("[ELF] ELF instruction set 'i386' is not supported.\n");
    } else if (elf_header->inst_set == 0x3E) {
        // found
    }

    // Check ELF version

    // Fetch program entry offset
    printf("Program header table count: %d\n", elf_header->program_hdr_table_entry_nbr);
    printf("Section header table count: %d\n", elf_header->section_hdr_table_entry_nbr);

    // Create program and section header table addresses
    program_header_addr = addr + elf_header->program_hdr_table_offset;
    section_header_addr = addr + elf_header->section_hdr_table_offset;
}

void elf_parse_section_header_table(uintptr_t elf_addr, uintptr_t section_table_addr) {
    // Make an address to the SHSTRTAB Table
    elf_section_table_t* section_shstrtab_table = (elf_section_table_t*)(section_table_addr + (0x40 * elf_header->section_index_shstrtab_table));

    // Loop through the section header table to find .text
    for (size_t i = 0; i < elf_header->section_hdr_table_entry_nbr; i++) {
        elf_section_table_t* elf_section_table = (elf_section_table_t*)(section_table_addr + (0x40 * i));

        // Check the name of the section against the offset in the shstrtab table (Is null terminated)
        uintptr_t shstrtab_section_name = elf_addr + section_shstrtab_table->section_data_offset + elf_section_table->section_name;
        if (test_strcmp((uint8_t*)shstrtab_section_name, ".text") == 0) {
            printf("[ELF] Found .text section.\n");
            elf_text_section = (elf_section_table_t*)(section_table_addr + (0x40 * i));
        } else if (test_strcmp((uint8_t*)shstrtab_section_name, ".rodata") == 0) {
            printf("[ELF] Found .rodata section.\n");
            elf_rodata_section = (elf_section_table_t*)(section_table_addr + (0x40 * i));
        }
    }
}

void elf_parse_program_header_table(uintptr_t addr) {
    //elf_program_table_t* elf_program_table = (elf_program_table_t*)addr;


    for (size_t i = 0; i < elf_header->program_hdr_table_entry_nbr; i++) {
        elf_program_table_t* elf_program_table = (elf_program_table_t*)(addr + (0x38 * i));
        printf("Segment type: %lx\n", elf_program_table->segment_type);
    }
}


int test_strcmp(const char *s1, const char *s2)
{
    while (*s1 == *s2)
    {
        if (*s1 == '\0')
        {
            return 0;
        }

        ++s1;
        ++s2;
    }

    return *s1 - *s2;
}
