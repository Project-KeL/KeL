#ifndef LEXER_ALLOCATION
#define LEXER_ALLOCATION

#include "lexer_def.h"

bool lexer_allocate_chunk(
	long int minimum,
	Lexer* lexer);

#endif
