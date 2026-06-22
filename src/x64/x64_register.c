#include <stdint.h>
#include <string.h>
#include "x64_register.h"

void initialize_regslots(RegSlots* regslots) {
	memset(
		regslots->pool.reg,
		(char) true,
		Reg_COUNT * sizeof(bool));
	memset(
		regslots->pool.stack,
		(char) true,
		512 * sizeof(bool));

	bool* regs = regslots->pool.reg;
	regs[Reg_NO] = false;
	regs[Reg_RBP] = false; // base pointer
	regs[Reg_RSP] = false; // stack pointer
}

Slot reg_alloc(RegSlots* regslots) {
	for(
	uint32_t i = 1;
	i < Reg_COUNT;
	i += 1) {
		if(regslots->pool.reg[i]) {
			regslots->pool.reg[i] = false;
			return (Slot) {
				.spilled = false,
				.slot = i};
		}
	}

	for(
	uint32_t i = 0;
	i < 512;
	i += 1) {
		if(regslots->pool.stack[i]) {
			regslots->pool.stack[i] = false;
			return (Slot) {
				.spilled = true,
				.slot = i};
		}
	}

	return (Slot) {
		.spilled = false,
		.slot = Reg_NO};
}

void reg_free(
Slot slot,
RegSlots* regslots) {
	if(slot.spilled)
		regslots->pool.stack[slot.slot] = true;
	else
		regslots->pool.reg [slot.slot] = true;
}
