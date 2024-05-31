#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
	void* addr;
	size_t size;
	size_t size_type;
} MemoryArea;

bool create_memory_area(
	size_t size,
	size_t size_type,
	MemoryArea* restrict memArea);
bool memory_area_realloc(
	size_t size,
	MemoryArea* restrict memArea);
void destroy_memory_area(MemoryArea* restrict memArea);

typedef struct MemoryChainLink MemoryChainLink;

struct MemoryChainLink {
	MemoryArea memArea;
	MemoryChainLink* next;
};

typedef struct {
	MemoryChainLink* first;
	size_t count;
	MemoryChainLink* current;
	MemoryChainLink* last;
} MemoryChain;

bool create_memory_chain(
	size_t size,
	size_t size_type,
	MemoryChain* restrict memChain);
bool memory_chain_add_area(
	size_t size,
	size_t size_type,
	MemoryChain* restrict memChain);
MemoryArea* memory_chain_get(MemoryChain* restrict memChain);
void memory_chain_next(MemoryChain* restrict memChain);
void memory_chain_rewind(MemoryChain* restrict memChain) ;
void destroy_memory_chain(MemoryChain* restrict memChain);

#endif
