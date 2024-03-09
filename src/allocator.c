#include "allocator.h"

bool create_allocator(
size_t size,
Allocator* restrict allocator) {
	allocator->last = NULL;
	allocator->size = 0;
	allocator->address = malloc(size * sizeof(uint8_t));
	
	if(allocator->address == NULL)
		return false;

	allocator->last = allocator->address;
	allocator->size = size;
	return true;
}

void destroy_allocator(Allocator* restrict allocator) {
	free(allocator->address);
	allocator->address = NULL;
	allocator->size = 0;
}

bool allocator_fit(
Allocator* restrict allocator,
size_t size) {
	return allocator->size - (size_t) (allocator->last - allocator->address) <= size;
}

bool allocator_push(
Allocator* restrict allocator,
size_t count) {
	if(allocator->size > SIZE_MAX - count)
		return false;
	else {
		allocator->last += count;
		allocator->size += count;
		return true;
	}
}

bool allocator_pop(
Allocator* restrict allocator,
size_t count) {
	if(allocator->size < count)
		return false;
	else {
		allocator->last -= count;
		allocator->size -= count;
		return true;
	}
}

void allocator_pop_all(Allocator* restrict allocator) {
	allocator_pop(
		allocator,
		allocator->size);
}
