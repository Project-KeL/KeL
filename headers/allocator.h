#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
	void* addr;
	size_t count;
	size_t size_type;
} MemoryArea;

void initialize_memory_area(MemoryArea* memArea);
bool create_memory_area(
	size_t count,
	size_t size_type,
	MemoryArea* memArea);
bool memory_area_realloc(
	size_t size,
	MemoryArea* memArea);
void destroy_memory_area(MemoryArea* memArea);

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

void initialize_memory_chain(MemoryChain* memChain);
bool create_memory_chain(
	size_t count,
	size_t size_type,
	MemoryChain* memChain);
void memory_chain_destroy_memory_area_last(MemoryChain* memChain);
void destroy_memory_chain(MemoryChain*  memChain);
bool memory_chain_add_area(
	size_t count,
	MemoryChain* memChain);
bool memory_chain_reserve_data(
	size_t count,
	MemoryChain* memChain);
void initialize_memory_chain_state(MemoryChainState* memChain_state);
void memory_chain_state_save(
	const MemoryChain* memChain,
	MemoryChainState* memChain_state);
void memory_chain_state_restore(
	MemoryChain* memChain,
	const MemoryChainState* memChain_state);

#endif
