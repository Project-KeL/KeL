#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "binary.h"
#include "../linker/elf.h"

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
	ELF_EHDR ehdr = create_elf_ehdr(
		ELF_E_TYPE_EXEC,
		0x08048078, // e_entry
		0x40, // e_phoff
		0x00, // e_shoff
		0x38, // e_phentsize
		0x01, // e_phnum
		0x00, // e_shentsize
		0x00, // e_shnum
		0x00); // e_shstrndx
	fwrite(
		&ehdr,
		sizeof(ELF_EHDR),
		1,
		binary->file);
	ELF_PHDR phdr = create_elf_phdr(
		ELF_P_TYPE_LOAD, // p_type
		ELF_P_FLAG_X | ELF_P_FLAG_W | ELF_P_FLAG_R, // p_flags
		0, // p_offset
		0x08048000, // p_vaddr
		0x08048000, // p_paddr
		0, // p_filesz: reserved
		0, // p_memsz: reserved
		0x1000); // p_align
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
