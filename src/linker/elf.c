#include <assert.h>
#include "elf.h"

ELF_EHDR create_elf_ehdr(
uint16_t e_type,
uint64_t e_entry,
uint64_t e_phoff,
uint64_t e_shoff,
uint16_t e_phentsize,
uint16_t e_phnum,
uint16_t e_shentsize,
uint16_t e_shnum,
uint16_t e_shstrndx) {
	static_assert(sizeof(ELF_EHDR) == 0x40);
	return (ELF_EHDR) {
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
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00},
		.e_type = e_type,
		.e_machine = 0x3E, // x64
		.e_version = 0x01,
		.e_entry = e_entry,
		.e_phoff = e_phoff,
		.e_shoff = e_shoff,
		.e_flags = 0,
		.e_ehsize = 0x40,
		.e_phentsize = e_phentsize,
		.e_phnum = e_phnum,
		.e_shentsize = e_shentsize,
		.e_shnum = e_shnum,
		.e_shstrndx = e_shstrndx
	};
}

ELF_PHDR create_elf_phdr(
uint32_t p_type,
uint32_t p_flags,
uint64_t p_offset,
uint64_t p_vaddr,
uint64_t p_paddr,
uint64_t p_filesz,
uint64_t p_memsz,
uint64_t p_align) {
	static_assert(sizeof(ELF_PHDR) == 0x38);
	return (ELF_PHDR) {
		.p_type = p_type,
		.p_flags = p_flags,
		.p_offset = p_offset,
		.p_vaddr = p_vaddr,
		.p_paddr = p_paddr,
		.p_filesz = p_filesz,
		.p_memsz = p_memsz,
		.p_align = p_align};
}
