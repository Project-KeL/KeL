#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
	void* addr;
	size_t count;
	size_t size_type;
} MemoryArea;

void initialize_memory_area(MemoryArea* restrict memArea);
bool create_memory_area(
	size_t count,
	size_t size_type,
	MemoryArea* restrict memArea);
bool memory_area_realloc(
	size_t size,
	MemoryArea* restrict memArea);
void destroy_memory_area(MemoryArea* restrict memArea);

typedef struct MemoryChainLink MemoryChainLink;

struct MemoryChainLink {
	MemoryArea memArea;
	MemoryChainLink* previous;
	MemoryChainLink* next;
};

typedef struct {
	MemoryChainLink* first;
	size_t count;
	MemoryChainLink* last;
	void* previous;
	void* top;
} MemoryChain;

typedef struct {
	size_t buffer_count;
	void* buffer_previous;
	void* buffer_top;
} MemoryChainState;

void initialize_memory_chain(MemoryChain* restrict memChain);
bool create_memory_chain(
	size_t count,
	size_t size_type,
	MemoryChain* restrict memChain);
bool memory_chain_add_area(
	size_t count,
	MemoryChain* restrict memChain);
void memory_chain_destroy_memory_area_last(MemoryChain* restrict memChain);
void destroy_memory_chain(MemoryChain* restrict memChain);
void initialize_memory_chain_state(MemoryChainState* restrict memChain_state);
void memory_chain_state_save(
	const MemoryChain* restrict memChain,
	MemoryChainState* restrict memChain_state);
void memory_chain_state_restore(
	MemoryChain* restrict memChain,
	const MemoryChainState* restrict memChain_state);

#endif
