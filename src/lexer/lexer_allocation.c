#include <stdlib.h>
#include <stdio.h>
#include "lexer_allocation.h"

#define CHUNK 4096
bool lexer_create_allocator(Lexer* lexer) {
	if(create_memory_area(
		CHUNK,
		sizeof(Token),
		&lexer->tokens)
	== false)
		return false;

	return true;
}

bool lexer_allocate_chunk(
size_t minimum,
Lexer* lexer) {
	if(lexer->tokens.count <= minimum) {
		const bool error = memory_area_realloc(
			(minimum / CHUNK + 1) * CHUNK,
			&lexer->tokens);
		return error;
	}

	return true;
}

bool lexer_allocator_shrink(Lexer* lexer) {
	const bool error = memory_area_realloc(
		lexer->tokens.count + 1, // null token
		&lexer->tokens);
	return error;
}

void lexer_destroy_allocator(Lexer* lexer) {
	destroy_memory_area(&lexer->tokens);
}
#undef CHUNK
