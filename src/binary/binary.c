#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "binary.h"

bool create_binary(
const char* restrict path,
Binary* restrict binary) {
	binary->path = NULL;

	binary->file = fopen(
		path,
		"w");

	if(binary->file == NULL) {
		printf("Cannot create the binary file.");
		destroy_binary(binary);
		return false;
	}

	binary->path = path;
	return true;
}

bool destroy_binary(Binary* binary) {
	if(binary->file != NULL
	&& fclose(binary->file) == EOF) {
		printf("Cannot close the binary.");
		return false;
	}

	binary->path = NULL;
	binary->file = NULL;
}

bool binary_append_byte(
Binary* restrict binary,
uint8_t byte) {
	return fwrite(
		&byte,
		1,
		1,
		binary->file) == 1;
}

bool binary_append_little_endian_word(
Binary* restrict binary,
uint16_t byte) {
	return fwrite(
		&byte,
		2,
		1,
		binary->file) == 1;
}

bool binary_append_big_endian_word(
Binary* restrict binary,
uint16_t byte) {
	return fwrite(
		&byte,
		1,
		2,
		binary->file) == 1;
}

#define LEN(array) (sizeof(array) / sizeof(array[0]))
#define APPEND_BYTE(byte) binary_append_byte( \
	binary, \
	byte)
#define APPEND_WORD(word) binary_append_big_endian_word( \
	binary, \
	word)

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

static void binary_x64_elf_initialize(Binary* restrict binary) {
	static_assert(sizeof(ELF_EHDR) == 0x40);
	static_assert(sizeof(ELF_PHDR) == 0x38);
	ELF_EHDR ehdr = {
		.e_ident = {
			0x7F,
			'E',
			'L',
			'F',
			0x02, // 64 bits
			0x01, // LSB
			0x01, // ELF version
			0x00, // OS ABI (ignore)
			0x00, // ABI version (ignore)
			// padding
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00},
		.e_type = 0x02, // 0x02: executable file
		.e_machine = 0x3E, // x64
		.e_version = 0x01,
		.e_entry = 0x08048078,
		.e_phoff = 0x40,
		.e_shoff = 0,
		.e_flags = 0,
		.e_ehsize = 0x40,
		.e_phentsize = 0x38,
		.e_phnum = 0x01,
		.e_shentsize = 0,
		.e_shnum = 0,
		.e_shstrndx = 0
	};

	fwrite(
		&ehdr,
		sizeof(ELF_EHDR),
		1,
		binary->file);

	ELF_PHDR phdr = {
		.p_type = 0x01, // PT_LOAD
		.p_flags = 0x01 | 0x02 | 0x04, // XWR
		.p_offset = 0,
		.p_vaddr = 0x08048000,
		.p_paddr = 0x08048000,
		.p_filesz = 0, // reserved
		.p_memsz = 0, // reserved
		.p_align = 0x1000
	};

	fwrite(
		&phdr,
		sizeof(ELF_PHDR),
		1,
		binary->file);
}

static void binary_x64_elf_terminate(Binary* restrict binary) {
	// insert the size of the file at p_filesz and p_memsz
	uint64_t file_size = ftell(binary->file);
	fseek(
		binary->file,
		0x60,
		SEEK_SET);
	fwrite(
		&file_size,
		1,
		8,
		binary->file);
	fwrite(
		&file_size,
		1,
		8,
		binary->file);
}

bool binary_x64(
Binary* restrict binary,
const Parser* restrict parser) {
	binary_x64_elf_initialize(binary);

	for(long int i = 0;
	i < parser->count - 1;
	++i) {
		const Node* restrict const node = &parser->nodes[i];

		if(node->type == NodeType_CORE_B) {
			APPEND_BYTE(node->value);
		}
	}

	binary_x64_elf_terminate(binary);
}

#undef APPEND_WORD
#undef APPEND_BYTE
