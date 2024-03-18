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

static void binary_x64_elf_initialize(Binary* restrict binary) {
	const uint8_t e_ident[] = {
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
		0x00};
	static_assert(LEN(e_ident) == 16);
	fwrite(
		e_ident,
		LEN(e_ident),
		1,
		binary->file);
	const uint16_t e_type = 0x02; // executable file
	fwrite(
		&e_type,
		1,
		2,
		binary->file);
	const uint16_t e_machine = 0x3E; // x64
	fwrite(
		&e_machine,
		1,
		2,
		binary->file);
	const uint32_t e_version = 1;
	fwrite(
		&e_version,
		1,
		4,
		binary->file);
	const uint64_t e_entry = 0x08048078; // reserved
	fwrite(
		&e_entry,
		1,
		8,
		binary->file);
	const uint64_t e_phoff = 0x40; // reserved
	fwrite(
		&e_phoff,
		1,
		8,
		binary->file);
	const uint64_t e_shoff = 0; // reserved
	fwrite(
		&e_shoff,
		1,
		8,
		binary->file);
	const uint32_t e_flags = 0; // ?
	fwrite(
		&e_flags,
		1,
		4,
		binary->file);
	const uint16_t e_ehsize = 0x40; // reserved
	fwrite(
		&e_ehsize,
		1,
		2,
		binary->file);
	const uint16_t e_phentsize = 0x38; // size per program header
	fwrite(
		&e_phentsize,
		1,
		2,
		binary->file);
	const uint16_t e_phnum = 1; // number of program headers
	fwrite(
		&e_phnum,
		1,
		2,
		binary->file);
	const uint16_t e_shentsize = 0; // size per section header
	fwrite(
		&e_shentsize,
		1,
		2,
		binary->file);
	const uint16_t e_shnum = 0; // number of section headers
	fwrite(
		&e_shnum,
		1,
		2,
		binary->file);
	const uint16_t e_shstrndx = 0; // section header string table index
	fwrite(
		&e_shstrndx,
		1,
		2,
		binary->file);
	// program header
	const uint32_t p_type = 0x01; // PT_LOAD
	fwrite(
		&p_type,
		1,
		4,
		binary->file);
	const uint32_t p_flags = 0x01 | 0x02 | 0x04; // XWR
	fwrite(
		&p_flags,
		1,
		4,
		binary->file);
	const uint64_t p_offset = 0; // offset of the segment
	fwrite(
		&p_offset,
		1,
		8,
		binary->file);
	const uint64_t p_vaddr = 0x08048000; // virtual address
	fwrite(
		&p_vaddr,
		1,
		8,
		binary->file);
	const uint64_t p_paddr = 0x08048000; // physical address
	fwrite(
		&p_paddr,
		1,
		8,
		binary->file);
	const uint64_t p_filesz = 0; // size of the segment in the file
	fwrite(
		&p_filesz,
		1,
		8,
		binary->file);
	const uint64_t p_memsz = 0; // size of the segment in memory
	fwrite(
		&p_memsz,
		1,
		8,
		binary->file);
	const uint64_t p_align = 0x1000;
	fwrite(
		&p_align,
		1,
		8,
		binary->file);
}

static void binary_x64_elf_terminate(Binary* restrict binary) {
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
