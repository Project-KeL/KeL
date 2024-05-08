#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "binary.h"
#include "elf.h"

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
	ELF_EHDR ehdr = (ELF_EHDR) {
		.e_ident = {
			[ELF_E_INDEX_MAGIC_0] = ELF_E_MAGIC_0,
			[ELF_E_INDEX_MAGIC_1] = ELF_E_MAGIC_1,
			[ELF_E_INDEX_MAGIC_2] = ELF_E_MAGIC_2,
			[ELF_E_INDEX_MAGIC_3] = ELF_E_MAGIC_3,
			[ELF_E_INDEX_CLASS] = ELF_E_CLASS_64,
			[ELF_E_INDEX_DATA] = ELF_E_DATA_LSB,
			[ELF_E_INDEX_VERSION] = ELF_E_VERSION_CURRENT,
			[ELF_E_INDEX_OS_ABI] = ELF_E_OS_ABI_NONE,
			[ELF_E_INDEX_ABI_VERSION] = ELF_E_ABI_VERSION_0},
		.e_type = ELF_E_TYPE_EXEC,
		.e_machine = ELF_E_MACHINE_x64, // x64
		.e_version = 1,
		.e_entry = 0x400078,
		.e_phoff = 0x40,
		.e_shoff = 0x00,
		.e_flags = 0,
		.e_ehsize = 0x40,
		.e_phentsize = 0x38,
		.e_phnum = 1,
		.e_shentsize = 0x00,
		.e_shnum = 0,
		.e_shstrndx = 0x00};
	fwrite(
		&ehdr,
		sizeof(ELF_EHDR),
		1,
		binary->file);
	ELF_PHDR phdr = {
		.p_type = ELF_P_TYPE_LOAD,
		.p_flags = ELF_P_FLAG_X | ELF_P_FLAG_W | ELF_P_FLAG_R,
		.p_offset = 0x00,
		.p_vaddr = 0x400000,
		.p_paddr = 0x400000,
		.p_filesz = 0,
		.p_memsz = 0,
		.p_align = 0};
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
		const Node* node = &parser->nodes[i];
/*
		if(node->type == NodeType_CORE_B) {
			APPEND_BYTE(node->value);
*/
	}

	binary_x64_elf_terminate(binary);
}

#undef APPEND_WORD
#undef APPEND_BYTE
