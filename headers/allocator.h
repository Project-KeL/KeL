#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
	void* base;
	size_t count;
	size_t size_type;
} MemoryArea;

typedef struct {
	const MemoryArea* memArea;
	size_t offset;
} MemoryAreaIterator;

void initialize_memory_area(MemoryArea* memArea);
bool create_memory_area(
	size_t count,
	size_t size_type,
	MemoryArea* memArea);
void destroy_memory_area(MemoryArea* memArea);
bool memory_area_realloc(
	size_t size,
	MemoryArea* memArea);

bool start_memory_area_iterator(
	const MemoryArea* memArea,
	MemoryAreaIterator* memAreaIt);
void end_memory_area_iterator(MemoryAreaIterator* memAreaIt);
void* memory_area_iterator_get(MemoryAreaIterator* memAreaIt);
bool memory_area_iterator_next(MemoryAreaIterator* memAreaIt);

typedef struct {
	MemoryArea memArea;
	void* top;
} MemoryStack;

typedef struct {
	void* top;
} MemoryStackState;

void initialize_memory_stack(MemoryStack* memStack);
bool create_memory_stack(
	size_t count,
	size_t size_type,
	MemoryStack* memStack);
void destroy_memory_stack(MemoryStack* memStack);
bool memory_stack_push(
	char* data,
	MemoryStack* stack);
bool memory_stack_discard(MemoryStack* memStack);
bool memory_stack_pop(
	char* data,
	MemoryStack* memStack);
void memory_stack_top(
	char* data,
	MemoryStack* memStack);
void* memory_stack_top_addr(MemoryStack* memStack);
bool memory_stack_realloc(
	size_t count,
	MemoryStack* memStack); // to implement
bool memory_stack_is_empty(MemoryStack* memStack);

void initialize_memory_stack_state(MemoryStackState* memStackState);
void memory_stack_state_save(
	MemoryStack* memStack,
	MemoryStackState* memStackState);
void memory_stack_state_restore(
	MemoryStack* memStack,
	MemoryStackState* memStackState);

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

typedef struct {
	MemoryChainLink* link;
	void* addr;
} MemoryChainIterator;

void initialize_memory_chain(MemoryChain* memChain);
bool create_memory_chain(
	size_t count,
	size_t size_type,
	MemoryChain* memChain);
void destroy_memory_chain(MemoryChain* memChain);
void memory_chain_destroy_memory_area_last(MemoryChain* memChain);
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

bool start_memory_chain_iterator(
	const MemoryChain* memChain,
	MemoryChainIterator* memChainIt);
void end_memory_chain_iterator(MemoryChainIterator* memChainIt);
void* memory_chain_iterator_get(MemoryChainIterator* memChainIt);
bool memory_chain_iterator_next(MemoryChainIterator* memChainIt);

#endif
