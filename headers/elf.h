#ifndef ELF_H
#define ELF_H

#include <stdint.h>

#define ELF_E_INDEX_MAGIC_0 0
#define ELF_E_INDEX_MAGIC_1 1
#define ELF_E_INDEX_MAGIC_2 2
#define ELF_E_INDEX_MAGIC_3 3
#define ELF_E_MAGIC_0 0x7F
#define ELF_E_MAGIC_1 'E'
#define ELF_E_MAGIC_2 'L'
#define ELF_E_MAGIC_3 'F'

#define ELF_E_INDEX_CLASS 4
#define ELF_E_CLASS_NONE 0
#define ELF_E_CLASS_32 1
#define ELF_E_CLASS_64 2

#define ELF_E_INDEX_DATA 5
#define ELF_E_DATA_NONE 0
#define ELF_E_DATA_LSB 1
#define ELF_E_DATA_MSB 2

#define ELF_E_INDEX_VERSION 6
#define ELF_E_VERSION_NONE 0
#define ELF_E_VERSION_CURRENT 1

#define ELF_E_INDEX_OS_ABI 7
#define ELF_E_OS_ABI_NONE 0

#define ELF_E_INDEX_ABI_VERSION 8
#define ELF_E_ABI_VERSION_0 0

#define ELF_E_INDEX_PADDING 9

#define ELF_E_TYPE_NONE 0
#define ELF_E_TYPE_REL 1
#define ELF_E_TYPE_EXEC 2
#define ELF_E_TYPE_DYN 3

#define ELF_E_MACHINE_x64 0x3E

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

#endif
