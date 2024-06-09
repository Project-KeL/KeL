#ifndef LEXER_H
#define LEXER_H

#include "allocator.h"
#include "lexer_def.h"
#include "source.h"

void initialize_lexer(Lexer* lexer);
bool create_lexer(
	const Source* source,
	MemoryArea* restrict memArea,
	Lexer* lexer);
void destroy_lexer(Lexer* lexer);

#endif
