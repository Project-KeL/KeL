#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
	uint8_t* address;
	uint8_t* last;
	size_t size;
} Allocator;

bool create_allocator(
	size_t size,
	Allocator* restrict allocator);
void destroy_allocator(Allocator* restrict allocator);
bool allocator_fit(
	Allocator* restrict allocator,
	size_t size);
bool allocator_push(
	Allocator* restrict allocator,
	size_t count);
bool allocator_pop(
	Allocator* restrict allocator,
	size_t count);
void allocator_pop_all(Allocator* restrict allocator);

#endif
