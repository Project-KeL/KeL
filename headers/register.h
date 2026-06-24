#ifndef REGISTER_H
#define REGISTER_H

#include <stddef.h>
#include "allocator.h"
#include "tac.h"
#include "tac_quadruple.h"

typedef struct {
	bool spilled;
	uint32_t slot; // on the stack or a Reg (spilled)
} Slot;

typedef struct {
	size_t start; // the QuadEntry in the QuadList
	size_t end;
	size_t offset_node;
	Slot slot;
} SlotLifetime;

typedef struct {
	bool* reg;
	bool* stack; // 64 bits alignment
} SlotPool;

typedef struct {
	SlotLifetime* lifetimes;
	MemoryStack stack_range;
	size_t count_reg;
	size_t count_spilled;
	SlotPool pool;
} RegSlots;

void initialize_regslots(RegSlots* regslots);
	bool create_regslots(
	size_t count_reg,
	size_t count_spilled,
	TAC* tac,
	RegSlots* regslots);
void destroy_regslots(RegSlots* regslots);

#endif
