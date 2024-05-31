#include <assert.h>
#include "allocator.h"

bool create_memory_area(
size_t size,
size_t size_type,
MemoryArea* restrict memArea) {
	memArea->size_type = (size_type + 7) & ~7; // 64-bit alignment
	memArea->addr = malloc(size * memArea->size_type);
	
	if(memArea->addr == NULL) {
		destroy_memory_area(memArea);
		return false;
	}

	memArea->size = size;
	return true;
}

bool memory_area_realloc(
size_t size,
MemoryArea* restrict memArea) {
	if(memArea->size <= size) {
		void* const addr_realloc = realloc(
			memArea->addr,
			size * memArea->size_type);

		if(addr_realloc == NULL) {
			destroy_memory_area(memArea);
			return false;
		}

		memArea->addr = addr_realloc;
		memArea->size = size;
	}

	return true;
}

void destroy_memory_area(MemoryArea* restrict memArea) {
	free(memArea->addr);
	memArea->addr = NULL;
	memArea->size = 0;
	memArea->size_type = 0;
}

bool create_memory_chain(
size_t size,
size_t size_type,
MemoryChain* restrict memChain) {
	memChain->count = 0;
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
	memChain->count += 1;
	memChain->current = memChain->first;
	memChain->last = memChain->first;
	return true;
ERROR2:
	free(memChain->first);
ERROR1:
	destroy_memory_chain(memChain);
	return false;
}

bool memory_chain_add_area(
size_t size,
size_t size_type,
MemoryChain* restrict memChain) {
	memChain->last->next = malloc(sizeof(MemoryChainLink));

	if(memChain->last->next == NULL)
		goto ERROR1;

	if(create_memory_area(
		size,
		size_type,
		&memChain->last->next->memArea)
	== false)
		goto ERROR2;

	memChain->count += 1;
	memChain->last = memChain->last->next;
	memChain->last->next = NULL;
	return true;
ERROR2:
	free(memChain->last->next);
ERROR1:
	destroy_memory_chain(memChain);
	return false;
}

MemoryArea* memory_chain_get(MemoryChain* restrict memChain) {
	return &memChain->current->memArea;
}

void memory_chain_next(MemoryChain* restrict memChain) {
	assert(memChain->current->next != NULL);
	memChain->current = memChain->current->next;
}

void memory_chain_rewind(MemoryChain* restrict memChain) {
	memChain->current = memChain->first;
}

void destroy_memory_chain(MemoryChain* restrict memChain) {
	MemoryChainLink* link = memChain->first;

	while(memChain->count != 0) {
		MemoryChainLink* buffer_link = link->next;
		destroy_memory_area(&link->memArea);
		free(link);
		link = buffer_link;
		memChain->count -= 1;
	}

	memChain->first = NULL;
	memChain->current = NULL;
	memChain->last = NULL;
}
