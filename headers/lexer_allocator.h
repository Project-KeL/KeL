#ifndef LEXER_ALLOCATION
#define LEXER_ALLOCATION

#include "lexer.h"

void lexer_initialize_allocator(Lexer* lexer);
bool lexer_create_allocator_chunk(Lexer* lexer);
bool lexer_create_allocator(Lexer* lexer);
bool lexer_create_allocator_limit(
	size_t limit,
	Lexer* lexer);
bool lexer_create_allocator_chunk(Lexer* lexer);
bool lexer_allocator_shrink(Lexer* lexer);
void lexer_destroy_allocator(Lexer* lexer);

#endif
