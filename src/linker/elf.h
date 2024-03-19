#ifndef ELF_H
#define ELF_H

#include <stdint.h>

#define ELF_E_TYPE_NONE 0x00
#define ELF_E_TYPE_REL 0x01
#define ELF_E_TYPE_EXEC 0x02
#define ELF_E_TYPE_DYN 0x03
#define ELF_E_TYPE_CORE 0x04

typedef struct {
	uint8_t e_ident[16];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint64_t e_entry;
	uint64_t e_phoff;
	uint64_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize; // size per program header 
	uint16_t e_phnum; // number of program headers
	uint16_t e_shentsize; // size per section header
	uint16_t e_shnum; // number of section headers
	uint16_t e_shstrndx; // section header string table index
} ELF_EHDR;

#define ELF_P_TYPE_NULL 0x00
#define ELF_P_TYPE_LOAD 0x01
#define ELF_P_TYPE_DYNAMIC 0x02
#define ELF_P_TYPE_INTERP 0x03
#define ELF_P_TYPE_NOTE 0x04
#define ELF_P_TYPE_PHDR 0x06
#define ELF_P_TYPE_TLS 0x07

#define ELF_P_FLAG_X 0x01
#define ELF_P_FLAG_W 0x02
#define ELF_P_FLAG_R 0x04

typedef struct {
	uint32_t p_type;
	uint32_t p_flags;
	uint64_t p_offset; // offset of the segment
	uint64_t p_vaddr; // virtual address
	uint64_t p_paddr; // physical address
	uint64_t p_filesz; // size of the segment in the file
	uint64_t p_memsz; // size of the segment in memory
	uint64_t p_align;
} ELF_PHDR;

ELF_EHDR create_elf_ehdr(
	uint16_t e_type,
	uint64_t e_entry,
	uint64_t e_phoff,
	uint64_t e_shoff,
	uint16_t e_phentsize,
	uint16_t e_phnum,
	uint16_t e_shentsize,
	uint16_t e_shnum,
	uint16_t e_shstrndx);
ELF_PHDR create_elf_phdr(
	uint32_t p_type,
	uint32_t p_flags,
	uint64_t p_offset,
	uint64_t p_vaddr,
	uint64_t p_paddr,
	uint64_t p_filesz,
	uint64_t p_memsz,
	uint64_t p_align);

#endif
