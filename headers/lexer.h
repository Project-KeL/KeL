#ifndef LEXER_H
#define LEXER_H

#include "allocator.h"
#include "lexer_def.h"
#include "source.h"

bool create_lexer(
	const Source* source,
	MemoryArea* restrict allocator,
	Lexer* restrict lexer);
void destroy_lexer(
	Lexer* restrict lexer);

#endif
