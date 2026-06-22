#ifndef ELF_REGISTER_H
#define ELF_REGISTER_H

#include <stddef.h>
#include "tac_quadruple.h"

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
	bool spilled;
	uint32_t slot; // on the stack or a Reg (spilled)
} Slot;

typedef struct {
	size_t start; // the QuadEntry in the QuadList
	size_t end;
	Slot slot;
} SlotLifetime;

typedef struct {
	bool reg[Reg_COUNT];
	bool stack[512]; // 64 bits alignment
} SlotPool;

typedef struct {
	SlotPool pool;
} RegSlots;

#endif
