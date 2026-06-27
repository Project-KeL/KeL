#include <stdio.h>
#include "debug_register.h"
#include "register.h"

#ifndef NDEBUG

void debug_print_x64_register(RegSlots* regslots) {
	printf("REGISTER ALLOCATION:\n");
	MemoryStack* stack = &regslots->stack_range;
	size_t count_range = ((char*) stack->top - (char*) stack->area.base) / sizeof(SlotLifetime);
	SlotLifetime* ranges = regslots->stack_range.area.base;

	for(
	size_t i = 0;
	i < count_range;
	i += 1) {
		SlotLifetime* interval = ranges + i;
		printf(
			"\tNode %zu [%zu, %zu]\t -> \t%s %u\n",
			interval->offset_node,
			interval->start,
			interval->end,
			interval->slot.spilled ? "S" : "R",
			interval->slot.slot);
	}

	printf(
		"\nNumber of intervals: %zu\n",
		count_range);
}

#endif
