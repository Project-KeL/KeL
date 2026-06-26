#include "x64_mapping.h"
#include "register.h"

void initialize_regmap(RegMap* regmap) {
	initialize_regslots(&regmap->regslots);
}

bool create_regmap(
TAC* tac,
RegMap* regmap) {
	if(create_regslots(
		REGMAP_COUNT_REG,
		REGMAP_COUNT_SPILLED,
		tac,
		&regmap->regslots)
	== false)
		return false;

	return true;
}

void destroy_regmap(RegMap* regmap) {
	if(regmap == NULL)
		return;

	destroy_regslots(&regmap->regslots);
}

Reg regmap_from_slot_to_physical(uint32_t slot) {
	switch(slot) {
	case 0: return Reg_NO;
	case 1: return Reg_RAX;
	case 2: return Reg_RBX;
	case 3: return Reg_RCX;
	case 4: return Reg_RDX;
	case 5: return Reg_RSI;
	case 6: return Reg_RDI;
	case 7: return Reg_R8;
	case 8: return Reg_R9;
	case 9: return Reg_R10;
	case 10: return Reg_R11;
	case 11: return Reg_R12;
	case 12: return Reg_R13;
	case 13: return Reg_R14;
	case 14: return Reg_R15;
	default: assert(false);
	}
}

uint32_t regmap_from_physical_to_slot(Reg reg) {
	switch(reg) {
	case Reg_NO: return 0;
	case Reg_RAX: return 1;
	case Reg_RBX: return 2;
	case Reg_RCX: return 3;
	case Reg_RDX: return 4;
	case Reg_RSI: return 5;
	case Reg_RDI: return 6;
	case Reg_R8: return 7;
	case Reg_R9: return 8;
	case Reg_R10: return 9;
	case Reg_R11: return 10;
	case Reg_R12: return 11;
	case Reg_R13: return 12;
	case Reg_R14: return 13;
	case Reg_R15: return 14;
	default: assert(false);
	}
}

const char* regmap_to_str(Reg reg) {
	switch(reg) {
	case Reg_RAX: return "rax";
	case Reg_RBX: return "rbx";
	case Reg_RCX: return "rcx";
	case Reg_RDX: return "rdx";
	case Reg_RSI: return "rsi";
	case Reg_RDI: return "rdi";
	case Reg_RBP: return "rbp";
	case Reg_RSP: return "rsp";
	case Reg_R8: return "r8";
	case Reg_R9: return "r9";
	case Reg_R10: return "r10";
	case Reg_R11: return "r11";
	case Reg_R12: return "r12";
	case Reg_R13: return "r13";
	case Reg_R14: return "r14";
	case Reg_R15: return "r15";
	default: return NULL;
	}
}
