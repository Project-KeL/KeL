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
	case 1: return Reg_RBX;
	case 2: return Reg_R12;
	case 3: return Reg_R13;
	case 4: return Reg_R14;
	case 5: return Reg_R15;
	case 6: return Reg_R11;
	case 7: return Reg_R10;
	case 8: return Reg_R9;
	case 9: return Reg_R8;
	case 10: return Reg_RSI;
	case 11: return Reg_RDI;
	case 12: return Reg_RCX;
	case 13: return Reg_RDX;
	case 14: return Reg_RAX;
	default: assert(false);
	}
}

uint32_t regmap_from_physical_to_slot(Reg reg) {
	switch(reg) {
	case Reg_NO: return 0;
	case Reg_RBX: return 1;
	case Reg_R12: return 2;
	case Reg_R13: return 3;
	case Reg_R14: return 4;
	case Reg_R15: return 5;
	case Reg_R11: return 6;
	case Reg_R10: return 7;
	case Reg_R9: return 8;
	case Reg_R8: return 9;
	case Reg_RSI: return 10;
	case Reg_RDI: return 11;
	case Reg_RCX: return 12;
	case Reg_RDX: return 13;
	case Reg_RAX: return 14;
	default: assert(false);
	}
}

const char* regmap_to_str(Reg reg) {
	switch(reg) {
	case Reg_RBX: return "rbx";
	case Reg_R12: return "r12";
	case Reg_R13: return "r13";
	case Reg_R14: return "r14";
	case Reg_R15: return "r15";
	case Reg_R11: return "r11";
	case Reg_R10: return "r10";
	case Reg_R9: return "r9";
	case Reg_R8: return "r8";
	case Reg_RSI: return "rsi";
	case Reg_RDI: return "rdi";
	case Reg_RCX: return "rcx";
	case Reg_RDX: return "rdx";
	case Reg_RAX: return "rax";
	case Reg_RBP: return "rbp";
	case Reg_RSP: return "rsp";
	default: return NULL;
	}
}

bool regmap_is_caller_saved(Reg reg) {
	switch(reg) {
	case Reg_R11:
	case Reg_R10:
	case Reg_R9:
	case Reg_R8:
	case Reg_RSI:
	case Reg_RDI:
	case Reg_RCX:
	case Reg_RDX:
	case Reg_RAX:
		return true;
	default: return false;
	}
}

bool regmap_caller_saved(
size_t i,
Reg reg,
const TAC* tac,
const RegMap* regmap) {
#ifndef NDEBUG
	if(!regmap_is_caller_saved(reg))
		assert(false); // useless case
#endif

	const SlotLifetime* lifetimes = regmap->regslots.lifetimes;

	for(
	size_t j = 1;
	j < tac->stab.parser->nodes.count - 1;
	j += 1) {
		const SlotLifetime* lifetime = lifetimes + j;

		if(lifetime->start == 0)
			continue;
		// the register does not pass through a PAL call and a PAL end
		if(lifetime->start >= i
		|| lifetime->end <= i)
			continue;

		if(regmap_from_slot_to_physical(lifetime->slot.slot) == reg)
			return true;
	}

	return false;
}
