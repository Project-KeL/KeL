#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "allocator.h"

void initialize_memory_area(MemoryArea* memArea) {
	assert(memArea != NULL);

	memArea->base = NULL;
	memArea->count = 0;
	memArea->size_type = 0;
}

bool create_memory_area(
size_t count,
size_t size_type,
MemoryArea* memArea) {
	assert(count != 0);
	assert(size_type != 0);
	assert(memArea != NULL);

	if(count == 0 || size_type == 0)
		return false;

	memArea->size_type = size_type;
	memArea->base = calloc(count, memArea->size_type);
	
	if(memArea->base == NULL) {
		destroy_memory_area(memArea);
		return false;
	}

	memArea->count = count;
	return true;
}

void destroy_memory_area(MemoryArea* memArea) {
	if(memArea == NULL)
		return;

	free(memArea->base);
	initialize_memory_area(memArea);
}

bool memory_area_realloc(
size_t count,
MemoryArea* memArea) {
	assert(count != 0);
	assert(memArea != NULL);

	void* const addr_realloc = realloc(
		memArea->base,
		count * memArea->size_type);

	if(addr_realloc == NULL) {
		destroy_memory_area(memArea);
		return false;
	}

	memArea->base = addr_realloc;
	memArea->count = count;
	return true;
}

bool start_memory_area_iterator(
const MemoryArea* memArea,
MemoryAreaIterator* memAreaIt) {
	assert(memArea != NULL);
	assert(memAreaIt != NULL);

	memAreaIt->memArea = memArea;
	return memArea->count != 0;
}

void end_memory_area_iterator(MemoryAreaIterator* memAreaIt) {
	assert(memAreaIt != NULL);

	memAreaIt->memArea = NULL;
	memAreaIt->offset = 0;
}

void* memory_area_iterator_get(MemoryAreaIterator* memAreaIt) {
	const MemoryArea* memArea = memAreaIt->memArea;
	return (char*) memArea->base + memArea->size_type * memAreaIt->offset;
}

bool memory_area_iterator_next(MemoryAreaIterator* memAreaIt) {
	assert(memAreaIt != NULL);

	const bool check_bounderies = memAreaIt->offset < memAreaIt->memArea->count;
	memAreaIt->offset += 1;
	return check_bounderies;
}

void initialize_memory_stack(MemoryStack* memStack) {
	assert(memStack != NULL);

	initialize_memory_area(&memStack->memArea);
	memStack->top = NULL;
}

bool create_memory_stack(
size_t count,
size_t size_type,
MemoryStack* memStack) {
	assert(memStack != NULL);

	bool ret = create_memory_area(
		count,
		size_type,
		&memStack->memArea);
	memStack->top = memStack->memArea.base;
	return ret;
}

void destroy_memory_stack(MemoryStack* memStack) {
	if(memStack == NULL)
		return;

	destroy_memory_area(&memStack->memArea);
	memStack->top = NULL;
}

bool memory_stack_push(
char* data,
MemoryStack* memStack) {
	assert(memStack != NULL);

	MemoryArea* memArea = &memStack->memArea;

	if((char*) memStack->top >= (char*) memArea->base + memArea->size_type * memArea->count)
		return false;

	memcpy(
		memStack->top,
		data,
		memStack->memArea.size_type);
	memStack->top = (char*) memStack->top + memStack->memArea.size_type;
	return true;
}

bool memory_stack_discard(MemoryStack* memStack) {
	assert(memStack != NULL);

	if((char*) memStack->top <= (char*) memStack->memArea.base)
		return false;

	memStack->top = (char*) memStack->top - memStack->memArea.size_type;
	return true;
}

bool memory_stack_pop(
char* data,
MemoryStack* memStack) {
	assert(memStack != NULL);

	if(!memory_stack_discard(memStack))
		return false;

	memcpy(
		data,
		memStack->top,
		memStack->memArea.size_type);
	return true;
}

void memory_stack_top(
char* data,
MemoryStack* memStack) {
	assert(memStack != NULL);

	memcpy(
		data,
		(char*) memStack->top - memStack->memArea.size_type,
		memStack->memArea.size_type);
}

void* memory_stack_top_addr(MemoryStack* memStack) {
	assert(memStack != NULL);

	return (char*) memStack->top - memStack->memArea.size_type;
}

bool memory_stack_is_empty(MemoryStack* memStack) {
	return memStack->top == memStack->memArea.base;
}

void initialize_memory_stack_state(MemoryStackState* memStackState) {
	assert(memStackState != NULL);

	memStackState->top = NULL;
}

void memory_stack_state_save(
MemoryStack* memStack,
MemoryStackState* memStackState) {
	assert(memStack != NULL);
	assert(memStackState != NULL);

	memStackState->top = memStack->top;
}

void memory_stack_state_restore(
MemoryStack* memStack,
MemoryStackState* memStackState) {
	assert(memStack != NULL);
	assert(memStackState != NULL);

	memStack->top = memStackState->top;
}

void initialize_memory_chain(MemoryChain* memChain) {
	assert(memChain != NULL);

	memChain->first = NULL;
	memChain->count = 0;
	memChain->last = NULL;
	memChain->previous = NULL;
	memChain->top = NULL;
}

bool create_memory_chain(
size_t size,
size_t size_type,
MemoryChain* memChain) {
	assert(size != 0);
	assert(size_type != 0);
	assert(memChain != NULL);

	if(size == 0 || size_type == 0)
		return false;

	memChain->first = malloc(sizeof(MemoryChainLink));

	if(memChain->first == NULL)
		goto ERROR1;

	if(create_memory_area(
		size,
		size_type,
		&memChain->first->memArea)
	== false)
		goto ERROR2;

	memChain->first->next = NULL;
	memChain->first->previous = NULL;
	memChain->count += 1;

	memChain->last = memChain->first;
	memChain->previous = NULL;
	memChain->top = memChain->first->memArea.base;
	return true;
ERROR2:
	free(memChain->first);
ERROR1:
	destroy_memory_chain(memChain);
	return false;
}

void destroy_memory_chain(MemoryChain* memChain) {
	if(memChain == NULL)
		return;

	while(memChain->count > 1)
		memory_chain_destroy_memory_area_last(memChain);

	destroy_memory_area(&memChain->first->memArea);
	free(memChain->first);
	initialize_memory_chain(memChain);
}

void memory_chain_destroy_memory_area_last(MemoryChain* memChain) {
	assert(memChain != NULL);
	assert(memChain->count > 1);

	destroy_memory_area(&memChain->last->memArea);
	memChain->last = memChain->last->previous;
	free(memChain->last->next);
	memChain->last->next = NULL;
	memChain->count -= 1;

	const size_t size_type = memChain->first->memArea.size_type;
	MemoryChainLink* const previous = memChain->last->previous;

	if(previous != NULL)
		memChain->previous = (char*) previous->memArea.base + size_type * (previous->memArea.count - 1);
	else
		memChain->previous = NULL;
}

bool memory_chain_add_area(
size_t count,
MemoryChain* memChain) {
	assert(count != 0);
	assert(memChain != NULL);

	const size_t size_type = memChain->first->memArea.size_type;
	memChain->last->next = malloc(sizeof(MemoryChainLink));

	if(memChain->last->next == NULL)
		goto ERROR1;

	if(create_memory_area(
		count,
		size_type,
		&memChain->last->next->memArea)
	== false)
		goto ERROR2;

	memChain->last->next->previous = memChain->last;
	memChain->count += 1;

	memChain->previous = (char*) memChain->last->memArea.base + size_type * (memChain->last->memArea.count - 1);
	memChain->last = memChain->last->next;
	memChain->top = memChain->last->memArea.base;
	memChain->last->next = NULL;
	return true;
ERROR2:
	free(memChain->last->next);
ERROR1:
	destroy_memory_chain(memChain);
	return false;
}

bool memory_chain_reserve_data(
size_t count,
MemoryChain* memChain) {
	assert(count != 0);
	assert(memChain != NULL);

	MemoryArea* restrict memArea = &memChain->last->memArea;
	const size_t current_count = memArea->count;

	if((char*) memArea->base + memArea->size_type * (current_count - 1) <= (char*) memChain->top) {
		// the remaining area is filled with blanks (calloc)
		if(memory_chain_add_area(
			count,
			memChain)
		== false)
			return false;
	} else {
		memChain->previous = memChain->top;
		memChain->top = (char*) memChain->top + memArea->size_type;
	}

	return true;
}

void initialize_memory_chain_state(MemoryChainState* memChain_state) {
	assert(memChain_state != NULL);

	memChain_state->buffer_count = 0;
	memChain_state->buffer_previous = NULL;
	memChain_state->buffer_top = NULL;
}

void memory_chain_state_save(
const MemoryChain* memChain,
MemoryChainState* memChain_state) {
	assert(memChain != NULL);
	assert(memChain_state != NULL);

	memChain_state->buffer_count = memChain->count;
	memChain_state->buffer_previous = memChain->previous;
	memChain_state->buffer_top = memChain->top;
}

void memory_chain_state_restore(
MemoryChain* memChain,
const MemoryChainState* memChain_state) {
	assert(memChain != NULL);
	assert(memChain_state != NULL);
	assert(memChain_state->buffer_count != 0);
	assert(memChain_state->buffer_top != NULL);

	while(memChain->count != memChain_state->buffer_count)
		memory_chain_destroy_memory_area_last(memChain);

	memChain->previous = memChain_state->buffer_previous;
	memChain->top = memChain_state->buffer_top;
}

bool start_memory_chain_iterator(
const MemoryChain* memChain,
MemoryChainIterator* memChainIt) {
	assert(memChain != NULL);
	assert(memChainIt != NULL);

	MemoryChainLink* const link = memChain->first;
	memChainIt->link = link;
	memChainIt->addr = link->memArea.base;
	return memChain->first != NULL;
}

void end_memory_chain_iterator(MemoryChainIterator* memChainIt) {
	assert(memChainIt != NULL);

	memChainIt->link = NULL;
	memChainIt->addr = NULL;
}

void* memory_chain_iterator_get(MemoryChainIterator* memChainIt) {
	assert(memChainIt != NULL);

	return memChainIt->addr;
}

bool memory_chain_iterator_next(MemoryChainIterator* memChainIt) {
	assert(memChainIt != NULL);

	MemoryChainLink* link = memChainIt->link;
	void* addr = memChainIt->addr;

	if((char*) addr == ((char*) link->memArea.base + link->memArea.size_type * (link->memArea.count - 1))) {
		if(link->next == NULL)
			return false;

		memChainIt->link = link->next;
		memChainIt->addr = link->memArea.base;
	} else {
		memChainIt->addr = (char*) addr + link->memArea.size_type;
	}

	return true;
}
