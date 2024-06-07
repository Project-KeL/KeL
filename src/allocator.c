#include <assert.h>
#include <stdio.h>
#include "allocator.h"

void initialize_memory_area(MemoryArea* restrict memArea) {
	memArea->addr = NULL;
	memArea->count = 0;
	memArea->size_type = 0;
}

bool create_memory_area(
size_t count,
size_t size_type,
MemoryArea* restrict memArea) {
	memArea->size_type = size_type;
	memArea->addr = calloc(count, memArea->size_type);
	
	if(memArea->addr == NULL) {
		destroy_memory_area(memArea);
		return false;
	}

	memArea->count = count;
	return true;
}

bool memory_area_realloc(
size_t count,
MemoryArea* restrict memArea) {
	void* const addr_realloc = realloc(
		memArea->addr,
		count * memArea->size_type);

	if(addr_realloc == NULL) {
		destroy_memory_area(memArea);
		return false;
	}

	memArea->addr = addr_realloc;
	memArea->count = count;
	return true;
}

void destroy_memory_area(MemoryArea* restrict memArea) {
	if(memArea == NULL)
		return;

	free(memArea->addr);
	initialize_memory_area(memArea);
}

void initialize_memory_chain(MemoryChain* restrict memChain) {
	memChain->first = NULL;
	memChain->count = 0;
	memChain->last = NULL;
	memChain->previous = NULL;
	memChain->top = NULL;
}

bool create_memory_chain(
size_t size,
size_t size_type,
MemoryChain* restrict memChain) {
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
	memChain->top = memChain->first->memArea.addr;
	return true;
ERROR2:
	free(memChain->first);
ERROR1:
	destroy_memory_chain(memChain);
	return false;
}

bool memory_chain_add_area(
size_t count,
MemoryChain* restrict memChain) {
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

	memChain->previous = (char*) memChain->last->memArea.addr + size_type * (memChain->last->memArea.count - 1);
	memChain->last = memChain->last->next;
	memChain->top = memChain->last->memArea.addr;
	memChain->last->next = NULL;
	return true;
ERROR2:
	free(memChain->last->next);
ERROR1:
	destroy_memory_chain(memChain);
	return false;
}

void memory_chain_destroy_memory_area_last(MemoryChain* restrict memChain) {
	assert(memChain->count > 1);
	destroy_memory_area(&memChain->last->memArea);

	memChain->last = memChain->last->previous;
	free(memChain->last->next);
	memChain->last->next = NULL;
	memChain->count -= 1;

	const size_t size_type = memChain->first->memArea.size_type;
	MemoryChainLink* const previous = memChain->last->previous;

	if(previous != NULL)
		memChain->previous = (char*) previous->memArea.addr + size_type * (previous->memArea.count - 1);
	else
		memChain->previous = NULL;
}

void destroy_memory_chain(MemoryChain* restrict memChain) {
	if(memChain == NULL)
		return;

	while(memChain->count > 1)
		memory_chain_destroy_memory_area_last(memChain);

	destroy_memory_area(&memChain->first->memArea);
	free(memChain->first);
	initialize_memory_chain(memChain);
}

void initialize_memory_chain_state(MemoryChainState* restrict memChain_state) {
	memChain_state->buffer_count = 0;
	memChain_state->buffer_previous = NULL;
	memChain_state->buffer_top = NULL;
}

void memory_chain_state_save(
const MemoryChain* restrict memChain,
MemoryChainState* restrict memChain_state) {
	memChain_state->buffer_count = memChain->count;
	memChain_state->buffer_previous = memChain->previous;
	memChain_state->buffer_top = memChain->top;
}

void memory_chain_state_restore(
MemoryChain* restrict memChain,
const MemoryChainState* restrict memChain_state) {
	assert(memChain_state->buffer_count != 0);
	assert(memChain_state->buffer_top != NULL);
	
	while(memChain->count != memChain_state->buffer_count)
		memory_chain_destroy_memory_area_last(memChain);

	memChain->previous = memChain_state->buffer_previous;
	memChain->top = memChain_state->buffer_top;
}
