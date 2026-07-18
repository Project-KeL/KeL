#ifndef BINARY_H
#define BINARY_H

#include <stdint.h>
#include <stdio.h>
#include "x64_mapping.h"

typedef struct {
	const char* path;
	FILE* file;
	long int offset_entry;
} Binary;

typedef enum: uint8_t {
	RegMod_ADDR = 0b00,
	RegMod_DISP_8 = 0b01,
	RegMod_DISP_16 = 0b10,
	RegMod_REG = 0b11,
} RegMod;

void initialize_binary(Binary* binary);
bool create_binary(
	const char* restrict path,
	Binary* restrict binary);
bool destroy_binary(Binary* restrict binary);
bool binary_append_byte(
	uint8_t byte,
	Binary* restrict binary);
void binary_x64_elf_initialize(Binary* binary);
void binary_x64_elf_terminate(Binary* binary);

void create_imm_u32_le(
	uint32_t u32,
	Binary* binary);
void create_u64_le(
	uint64_t u64,
	Binary* binary);

uint8_t create_modrm(
	RegMod mod,
	uint8_t rm,
	uint8_t reg);

void create_alu_r64_r64(
	uint8_t opcode,
	Reg dst,
	Reg src,
	Binary* binary);
void create_alu_r64_imm32(
	uint8_t digit,
	Reg dst,
	uint32_t imm32,
	Binary* binary);

void create_mov_r64_r64(
	Reg dst,
	Reg src,
	Binary* binary);
void create_syscall(Binary* binary);
void create_mov_r64_imm64(
	Reg dst,
	uint64_t imm64,
	Binary* binary);

#endif
