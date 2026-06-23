#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "allocator.h"

void initialize_memory_area(MemoryArea* area) {
	assert(area != NULL);

	area->base = NULL;
	area->count = 0;
	area->size_type = 0;
}

bool create_memory_area(
size_t count,
size_t size_type,
MemoryArea* area) {
	assert(count != 0);
	assert(size_type != 0);
	assert(area != NULL);

	area->size_type = size_type;
	area->base = calloc(count, size_type);
	
	if(area->base == NULL) {
		destroy_memory_area(area);
		return false;
	}

	area->count = count;
	return true;
}

void destroy_memory_area(MemoryArea* area) {
	if(area == NULL)
		return;

	free(area->base);
	initialize_memory_area(area);
}

bool memory_area_realloc(
size_t count,
MemoryArea* area) {
	assert(count != 0);
	assert(area != NULL);

	void* const addr_realloc = realloc(
		area->base,
		count * area->size_type);

	if(addr_realloc == NULL) {
		destroy_memory_area(area);
		return false;
	}

	area->base = addr_realloc;
	area->count = count;
	return true;
}

bool start_memory_area_iterator(
const MemoryArea* area,
MemoryAreaIterator* area_it) {
	assert(area != NULL);
	assert(area_it != NULL);

	area_it->area = area;
	return area->count != 0;
}

void end_memory_area_iterator(MemoryAreaIterator* area_it) {
	assert(area_it != NULL);

	area_it->area = NULL;
	area_it->offset = 0;
}

void* memory_area_iterator_get(MemoryAreaIterator* area_it) {
	assert(area_it != NULL);

	const MemoryArea* area = area_it->area;
	return (char*) area->base + area_it->offset * area->size_type;
}

bool memory_area_iterator_next(MemoryAreaIterator* area_it) {
	assert(area_it != NULL);

	const bool check_bounderies = area_it->offset < area_it->area->count - 1;

	if(check_bounderies)
		area_it->offset += 1;

	return check_bounderies;
}

void initialize_memory_stack(MemoryStack* stack) {
	assert(stack != NULL);

	initialize_memory_area(&stack->area);
	stack->top = NULL;
}

bool create_memory_stack(
size_t count,
size_t size_type,
MemoryStack* stack) {
	assert(stack != NULL);

	bool ret = create_memory_area(
		count,
		size_type,
		&stack->area);
	stack->top = stack->area.base;
	return ret;
}

void destroy_memory_stack(MemoryStack* stack) {
	if(stack == NULL)
		return;

	destroy_memory_area(&stack->area);
	stack->top = NULL;
}

bool memory_stack_realloc(
size_t count,
MemoryStack* stack) {
	assert(stack != NULL);

	size_t offset = (char*) stack->top - (char*) stack->area.base;

	if(memory_area_realloc(
		count,
		&stack->area)
	== false) {
		destroy_memory_stack(stack);
		return false;
	}

	stack->top = (char*) stack->area.base + offset;
	return true;
}

bool memory_stack_push(
char* data,
MemoryStack* stack) {
	assert(data != NULL);
	assert(stack != NULL);

	MemoryArea* area = &stack->area;

	if((char*) stack->top >= (char*) area->base + area->size_type * area->count)
		return false;

	memcpy(
		stack->top,
		data,
		stack->area.size_type);
	stack->top = (char*) stack->top + stack->area.size_type;
	return true;
}

bool memory_stack_discard(MemoryStack* stack) {
	assert(stack != NULL);

	if((char*) stack->top <= (char*) stack->area.base)
		return false;

	stack->top = (char*) stack->top - stack->area.size_type;
	return true;
}

bool memory_stack_pop(
char* data,
MemoryStack* stack) {
	assert(stack != NULL);

	if(!memory_stack_discard(stack))
		return false;

	memcpy(
		data,
		stack->top,
		stack->area.size_type);
	return true;
}

void memory_stack_top(
char* data,
MemoryStack* stack) {
	assert(stack != NULL);

	memcpy(
		data,
		(char*) stack->top - stack->area.size_type,
		stack->area.size_type);
}

void* memory_stack_top_addr(MemoryStack* stack) {
	assert(stack != NULL);

	return (char*) stack->top - stack->area.size_type;
}

bool memory_stack_is_empty(MemoryStack* stack) {
	return stack->top == stack->area.base;
}

void initialize_memory_stack_state(MemoryStackState* stack_state) {
	assert(stack_state != NULL);

	stack_state->top = NULL;
}

void memory_stack_state_save(
MemoryStack* stack,
MemoryStackState* stack_state) {
	assert(stack != NULL);
	assert(stack_state != NULL);

	stack_state->top = stack->top;
}

void memory_stack_state_restore(
MemoryStack* stack,
MemoryStackState* stack_state) {
	assert(stack != NULL);
	assert(stack_state != NULL);

	stack->top = stack_state->top;
}

void initialize_memory_chain(MemoryChain* chain) {
	assert(chain != NULL);

	chain->first = NULL;
	chain->count = 0;
	chain->last = NULL;
	chain->previous = NULL;
	chain->top = NULL;
}

bool create_memory_chain(
size_t size,
size_t size_type,
MemoryChain* chain) {
	assert(size != 0);
	assert(size_type != 0);
	assert(chain != NULL);

	chain->first = malloc(sizeof(MemoryChainLink));
	chain->count = 1; // to prevent issue with chain->count > 1 (destroy)

	if(chain->first == NULL)
		goto ERROR;

	if(create_memory_area(
		size,
		size_type,
		&chain->first->area)
	== false)
		goto ERROR;

	chain->first->next = NULL;
	chain->first->previous = NULL;

	chain->last = chain->first;
	chain->previous = NULL;
	chain->top = chain->first->area.base;
	return true;
ERROR:
	free(chain->first);
	destroy_memory_chain(chain);
	return false;
}

void destroy_memory_chain(MemoryChain* chain) {
	if(chain == NULL)
		return;

	while(chain->count > 1)
		memory_chain_destroy_memory_area_last(chain);

	destroy_memory_area(&chain->first->area);
	free(chain->first);
	initialize_memory_chain(chain);
}

void memory_chain_destroy_memory_area_last(MemoryChain* chain) {
	assert(chain != NULL);
	assert(chain->count > 1);

	destroy_memory_area(&chain->last->area);
	chain->last = chain->last->previous;
	free(chain->last->next);
	chain->last->next = NULL;
	chain->count -= 1;

	const size_t size_type = chain->first->area.size_type;
	MemoryChainLink* const previous = chain->last->previous;

	if(previous != NULL)
		chain->previous = (char*) previous->area.base + size_type * (previous->area.count - 1);
	else
		chain->previous = NULL;
}

bool memory_chain_add_area(
size_t count,
MemoryChain* chain) {
	assert(count != 0);
	assert(chain != NULL);

	const size_t size_type = chain->first->area.size_type;
	chain->last->next = malloc(sizeof(MemoryChainLink));

	if(chain->last->next == NULL)
		goto ERROR1;

	if(create_memory_area(
		count,
		size_type,
		&chain->last->next->area)
	== false)
		goto ERROR2;

	chain->last->next->previous = chain->last;
	chain->count += 1;

	chain->previous = (char*) chain->last->area.base + size_type * (chain->last->area.count - 1);
	chain->last = chain->last->next;
	chain->top = chain->last->area.base;
	chain->last->next = NULL;
	return true;
ERROR2:
	free(chain->last->next);
ERROR1:
	destroy_memory_chain(chain);
	return false;
}

bool memory_chain_reserve_data(
size_t count,
MemoryChain* chain) {
	assert(count != 0);
	assert(chain != NULL);

	MemoryArea* restrict memArea = &chain->last->area;
	const size_t current_count = memArea->count;

	if((char*) memArea->base + memArea->size_type * (current_count - 1) <= (char*) chain->top) {
		// the remaining area is filled with blanks (calloc)
		if(memory_chain_add_area(
			count,
			chain)
		== false)
			return false;
	} else {
		chain->previous = chain->top;
		chain->top = (char*) chain->top + memArea->size_type;
	}

	return true;
}

void initialize_memory_chain_state(MemoryChainState* chain_state) {
	assert(chain_state != NULL);

	chain_state->buffer_count = 0;
	chain_state->buffer_previous = NULL;
	chain_state->buffer_top = NULL;
}

void memory_chain_state_save(
const MemoryChain* chain,
MemoryChainState* chain_state) {
	assert(chain != NULL);
	assert(chain_state != NULL);

	chain_state->buffer_count = chain->count;
	chain_state->buffer_previous = chain->previous;
	chain_state->buffer_top = chain->top;
}

void memory_chain_state_restore(
MemoryChain* chain,
const MemoryChainState* chain_state) {
	assert(chain != NULL);
	assert(chain_state != NULL);
	assert(chain_state->buffer_count != 0);
	assert(chain_state->buffer_top != NULL);

	while(chain->count != chain_state->buffer_count)
		memory_chain_destroy_memory_area_last(chain);

	chain->previous = chain_state->buffer_previous;
	chain->top = chain_state->buffer_top;
}

bool start_memory_chain_iterator(
const MemoryChain* chain,
MemoryChainIterator* chain_it) {
	assert(chain != NULL);
	assert(chain_it != NULL);

	MemoryChainLink* const link = chain->first;
	chain_it->link = link;
	chain_it->addr = link->area.base;
	return chain->first != NULL;
}

void end_memory_chain_iterator(MemoryChainIterator* chain_it) {
	assert(chain_it != NULL);

	chain_it->link = NULL;
	chain_it->addr = NULL;
}

void* memory_chain_iterator_get(MemoryChainIterator* chain_it) {
	assert(chain_it != NULL);

	return chain_it->addr;
}

bool memory_chain_iterator_next(MemoryChainIterator* chain_it) {
	assert(chain_it != NULL);

	MemoryChainLink* link = chain_it->link;
	void* addr = chain_it->addr;

	if((char*) addr == ((char*) link->area.base + link->area.size_type * (link->area.count - 1))) {
		if(link->next == NULL)
			return false;

		chain_it->link = link->next;
		chain_it->addr = link->area.base;
	} else {
		chain_it->addr = (char*) addr + link->area.size_type;
	}

	return true;
}
