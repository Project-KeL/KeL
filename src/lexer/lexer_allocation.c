#include <stdlib.h>
#include "lexer_allocation.h"

bool lexer_allocate_chunk(
long int minimum,
Lexer* lexer) {
#define TOKENS_CHUNK 4096
	if(lexer->count <= minimum) {
		const long int reserve = minimum / TOKENS_CHUNK + 1;
		Token* tokens_realloc = realloc(
			lexer->tokens,
			reserve * TOKENS_CHUNK * sizeof(Token));

		if(tokens_realloc == NULL)
			return false;
		
		lexer->tokens = tokens_realloc;
		lexer->count = reserve * TOKENS_CHUNK;
	}
	
	return true;
#undef TOKENS_CHUNK
}
