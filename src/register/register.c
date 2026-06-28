#include <stdint.h>
#include <string.h>
#include "register.h"
#include "allocator.h"
#include "tac_quadruple.h"
#include <stdio.h>

static Slot create_slot_null(void) {
	return (Slot) {
		.spilled = false,
		.slot = 0};
}

static Slot regslots_alloc(RegSlots* regslots) {
	for(
	uint32_t i = 1;
	i <= regslots->count_reg;
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
	i < regslots->count_spilled;
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
		.slot = 0};
}

static void regslots_free(
Slot slot,
RegSlots* regslots) {
	if(slot.spilled)
		regslots->pool.stack[slot.slot] = true;
	else
		regslots->pool.reg [slot.slot] = true;
}
void initialize_regslots(RegSlots* regslots) {
	regslots->lifetimes = NULL;
	initialize_memory_stack(&regslots->stack_range);
	regslots->pool.reg = NULL;
	regslots->pool.stack = NULL;
}

bool create_regslots(
size_t count_reg,
size_t count_spilled,
TAC* tac,
RegSlots* regslots) {
	assert(tac != NULL);
	assert(regslots != NULL);

	bool error = false;
	error = (regslots->lifetimes = calloc(
		tac->stab.parser->nodes.count,
		sizeof(SlotLifetime))) == NULL;
	error |= (create_memory_stack(
		tac->quadlist.quadruples.area.count,
		sizeof(SlotLifetime),
		&regslots->stack_range)) == false;
	error |= (regslots->pool.reg = malloc((count_reg + 1) * sizeof(bool))) == NULL;
	error |= (regslots->pool.stack = malloc(count_spilled * sizeof(bool))) == NULL;
	regslots->count_reg = count_reg;
	regslots->count_spilled = count_spilled;

	if(error)
		goto END;

	*regslots->pool.reg = false;
	memset(
		regslots->pool.reg + 1,
		(char) true,
		count_reg * sizeof(bool));
	memset(regslots->pool.stack,
		(char) true,
		count_spilled * sizeof(bool));

	QuadList* quadlist = &tac->quadlist;

	for(
	size_t i = 1;
	i < quadlist->quadruples.area.count - 1;
	i += 1) {
		QuadEntry* entry = (QuadEntry*) quadlist->quadruples.area.base + i;
		// a temporary is always declared at .dst
		if(entry->dst.type == QuadItemType_TEMP
		|| entry->dst.type == QuadItemType_KEY) {
			size_t n = entry->dst.offset_node;

			if(regslots->lifetimes[n].start != 0) {
				memory_stack_push(
					(char*)(regslots->lifetimes + n),
					&regslots->stack_range);
			}

			regslots->lifetimes[n] = (SlotLifetime) {
				.start = i,
				.end = i,
				.offset_node = n,
				.slot = create_slot_null()};
		}

		if(entry->src1.type == QuadItemType_TEMP
		|| entry->src1.type == QuadItemType_KEY) {
			size_t n = entry->src1.offset_node;
			regslots->lifetimes[n].end = i;
		}

		if(entry->src2.type == QuadItemType_TEMP
		|| entry->src2.type == QuadItemType_KEY) {
			size_t n = entry->src2.offset_node;
			regslots->lifetimes[n].end = i;
		}
	}

	for(
	size_t i = 1;
	i < tac->stab.parser->nodes.count - 1;
	i += 1) {
		SlotLifetime* lifetime_close = regslots->lifetimes + i;

		if(lifetime_close->start != 0) {
			memory_stack_push(
				(char*) lifetime_close,
				&regslots->stack_range);
		}
	}

	MemoryStack* stack = &regslots->stack_range;
	size_t count_range = ((char*) stack->top - (char*) stack->area.base) / sizeof(SlotLifetime);
	SlotLifetime* ranges = regslots->stack_range.area.base;
	// insertion sort (for intervals, early start at first)
	for(
	size_t i = 1;
	i < count_range;
	i += 1) {
		SlotLifetime key = ranges[i];
		size_t j = i;

		while(j != 0
		   && ranges[j - 1].start
		> key.start) {
			ranges[j] = ranges[j - 1];
			j -= 1;
		}

		ranges[j] = key;
	}

	SlotLifetime** lifetime_active = malloc(count_range * sizeof(SlotLifetime*));

	if(lifetime_active == NULL) {
		error = true;
		goto END;
	}

	size_t count_active = 0;

	for(
	size_t i = 0;
	i < count_range;
	i += 1) {
		SlotLifetime* interval = ranges + i;
		size_t k = 0;

		while(k < count_active) {
			if(lifetime_active[k]->end < interval->start) {
				regslots_free(
					lifetime_active[k]->slot,
					regslots);
				lifetime_active[k] = lifetime_active[count_active - 1];
				count_active -= 1;
			} else {
				k += 1;
			}
		}

		interval->slot = regslots_alloc(regslots);

		if(interval->slot.spilled == false
		&& interval->slot.slot == 0) {
			error = true;
			break;
		}

		regslots->lifetimes[interval->offset_node].slot = interval->slot;
		lifetime_active[count_active] = interval;
		count_active += 1;
	}

	free(lifetime_active);
END:
	if(error) {
		destroy_regslots(regslots);
		return false;
	}

	return true;
}

void destroy_regslots(RegSlots* regslots) {
	if(regslots == NULL)
		return;

	free(regslots->pool.stack);
	free(regslots->pool.reg);
	destroy_memory_stack(&regslots->stack_range);
	free(regslots->lifetimes);
	initialize_regslots(regslots);
}
