#include <stdint.h>
#include <stdio.h>
#include "elf.h"
#include "x64_mapping.h"
#include "elf_binary.h"

void initialize_binary(Binary* binary) {
	binary->path = NULL;
	binary->file = NULL;
}

bool create_binary(
const char* restrict path,
Binary* restrict binary) {
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
	return true;
}

bool binary_append_byte(
uint8_t byte,
Binary* restrict binary) {
	return fwrite(
		&byte,
		1,
		1,
		binary->file) == 1;
}

static void binary_x64_elf_initialize(
const Assembly* assembly,
Binary* binary) {
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

static void binary_x64_elf_terminate(
const Assembly* assembly,
Binary* binary) {
	long int file_size = ftell(binary->file);
	// insert the size of the file at p_filesz and p_memsz
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

static void create_imm_u32_le(
uint32_t u32,
Binary* binary) {
	binary_append_byte(
		u32 & 0xFF,
		binary);
	binary_append_byte(
		(u32 >> 8) & 0xFF,
		binary);
	binary_append_byte(
		(u32 >> 16) & 0xFF,
		binary);
	binary_append_byte(
		(u32 >> 24) & 0xFF,
		binary);
}

static void create_u64_le(
uint64_t u64,
Binary* binary) {
	create_imm_u32_le(
		(uint32_t) u64,
		binary);
	create_imm_u32_le(
		(uint32_t)(u64 >> 32),
		binary);
}

static uint8_t create_modrm(
RegMod mod,
Reg rm,
Reg reg) {
	uint8_t x64_rm = regmap_from_physical_to_x64(rm);
	uint8_t x64_reg = regmap_from_physical_to_x64(reg);
	return (((uint8_t) mod) << 6) | ((x64_reg & 0x07) << 3) | (x64_rm & 0x07);
}

static void create_mov_r64_r64(
Reg dst,
Reg src,
Binary* binary) {
	uint8_t x64_dst = regmap_from_physical_to_x64(dst);
	uint8_t x64_src = regmap_from_physical_to_x64(src);
	uint8_t rex = 0x48
		| ((x64_src & 0x08) >> 1)
		| ((x64_dst & 0x08) >> 3);
	binary_append_byte(
		rex,
		binary);
	binary_append_byte(
		0x89,
		binary);
	binary_append_byte(
		create_modrm(
			RegMod_REG,
			dst,
			src),
		binary);
}

static void create_syscall(Binary* binary) {
	binary_append_byte(
		0x0F,
		binary);
	binary_append_byte(
		0x05,
		binary);
}

bool binary_x64_elf_write(
const Assembly* assembly,
Binary* binary) {
	binary_x64_elf_initialize(
		assembly,
		binary);
	static const uint8_t exit[] = {
		0xBF, 0x2A, 0x00, 0x00, 0x00,
		0xB8, 0x3C, 0x00, 0x00, 0x00,
		0x0F, 0x05};
	fwrite(exit, 1, sizeof(exit), binary->file);
	binary_x64_elf_terminate(
		assembly,
		binary);
	return true;
}
