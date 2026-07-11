#ifndef BINARY_H
#define BINARY_H

#include <stdint.h>
#include <stdio.h>
#include "assembly.h"

typedef struct {
	const char* path;
	FILE* file;
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
bool binary_x64_elf_write(
	const Assembly* assembly,
	Binary* binary);

#endif
