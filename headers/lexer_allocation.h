#ifndef LEXER_ALLOCATION
#define LEXER_ALLOCATION

#include "lexer_def.h"

bool lexer_create_allocator(Lexer* lexer);
bool lexer_allocate_chunk(
	size_t minimum,
	Lexer* lexer);
bool lexer_allocator_shrink(Lexer* lexer);
void lexer_destroy_allocator(Lexer* lexer);

#endif
