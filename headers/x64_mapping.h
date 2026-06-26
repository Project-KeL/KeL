#ifndef X64_MAPPING_H
#define X64_MAPPING_H

#include <stdint.h>
#include "register.h"

#define REGMAP_COUNT_REG 14
#define REGMAP_COUNT_SPILLED 512

typedef enum: uint32_t {
	Reg_NO = 0,
	Reg_RAX,
	Reg_RBX,
	Reg_RCX,
	Reg_RDX,
	Reg_RSI,
	Reg_RDI,
	Reg_RBP,
	Reg_RSP,
	Reg_R8,
	Reg_R9,
	Reg_R10,
	Reg_R11,
	Reg_R12,
	Reg_R13,
	Reg_R14,
	Reg_R15,
	Reg_COUNT,
} Reg;

typedef struct {
	RegSlots regslots;
} RegMap;

void initialize_regmap(RegMap* regmap);
bool create_regmap(
	TAC* tac,
	RegMap* regmap);
void destroy_regmap(RegMap* regmap);
Reg regmap_from_slot_to_physical(uint32_t slot);
uint32_t regmap_from_physical_to_slot(Reg reg);
const char* regmap_to_str(Reg reg);

#endif
