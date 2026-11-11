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
	const MemoryArea* area;
	size_t offset;
} MemoryAreaIterator;

static inline MemoryArea setup_memory_area(void) {
	return (MemoryArea) {};
}
bool create_memory_area(
	size_t count,
	size_t size_type,
	MemoryArea* area);
void destroy_memory_area(MemoryArea* area);
bool memory_area_realloc(
	size_t size,
	MemoryArea* area);

bool start_memory_area_iterator(
	const MemoryArea* area,
	MemoryAreaIterator* areaIt);
void end_memory_area_iterator(MemoryAreaIterator* areaIt);
void* memory_area_iterator_get(MemoryAreaIterator* areaIt);
bool memory_area_iterator_next(MemoryAreaIterator* areaIt);

typedef struct {
	MemoryArea area;
	void* top;
} MemoryStack;

typedef struct {
	void* top;
} MemoryStackState;

static inline MemoryStack setup_memory_stack(void) {
	return (MemoryStack) {
		.area = setup_memory_area()};
}
bool create_memory_stack(
	size_t count,
	size_t size_type,
	MemoryStack* stack);
void destroy_memory_stack(MemoryStack* stack);
bool memory_stack_realloc(
	size_t count,
	MemoryStack* stack);
bool memory_stack_push(
	char* data,
	MemoryStack* stack);
bool memory_stack_discard(MemoryStack* stack);
bool memory_stack_pop(
	char* data,
	MemoryStack* stack);
void memory_stack_top(
	char* data,
	MemoryStack* stack);
void* memory_stack_top_addr(MemoryStack* stack);
bool memory_stack_is_empty(MemoryStack* stack);

static inline MemoryStackState setup_memory_stack_state(void) {
	return (MemoryStackState) {};
}
void memory_stack_state_save(
	MemoryStack* stack,
	MemoryStackState* memStackState);
void memory_stack_state_restore(
	MemoryStack* stack,
	MemoryStackState* memStackState);

typedef struct MemoryChainLink MemoryChainLink;

struct MemoryChainLink {
	MemoryArea area;
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

static inline MemoryChain setup_memory_chain(void) {
	return (MemoryChain) {};
}
bool create_memory_chain(
	size_t count,
	size_t size_type,
	MemoryChain* chain);
void destroy_memory_chain(MemoryChain* chain);
void memory_chain_destroy_memory_area_last(MemoryChain* chain);
bool memory_chain_add_area(
	size_t count,
	MemoryChain* chain);
bool memory_chain_reserve_data(
	size_t count,
	MemoryChain* chain);

static inline MemoryChainState setup_memory_chain_state(void) {
	return (MemoryChainState) {};
}
void memory_chain_state_save(
	const MemoryChain* chain,
	MemoryChainState* chain_state);
void memory_chain_state_restore(
	MemoryChain* chain,
	const MemoryChainState* chain_state);

bool start_memory_chain_iterator(
	const MemoryChain* chain,
	MemoryChainIterator* chain_it);
void end_memory_chain_iterator(MemoryChainIterator* chain_it);
void* memory_chain_iterator_get(MemoryChainIterator* chain_it);
bool memory_chain_iterator_next(MemoryChainIterator* chain_it);

#endif
