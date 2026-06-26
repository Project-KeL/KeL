#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include <stdint.h>
#include <stdio.h>
#include "tac_quadruple.h"
#include "x64_mapping.h"

typedef struct {
	const char* path;
	FILE* file;
	const TAC* tac;
	const RegMap* regmap;
} Assembly;

void initialize_assembly(Assembly* assembly);
bool create_assembly(
	const char* restrict path,
	const TAC* tac,
	const RegMap* regmap,
	Assembly* restrict assembly);
bool destroy_assembly(Assembly* restrict assembly);
bool assembly_file_write(Assembly* assembly);

#endif
