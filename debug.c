#ifndef NDEBUG
#include <stdio.h>
#include "debug.h"

void debug_print_tokens(const Lexer* lexer) {
	const char* code = lexer->source->content;

	for(long int i = 0; i < lexer->count - 1; i += 1) {
		printf(
			"%d : %.*s (\"%.*s\", \"%.*s\") {%d, %d, %d}\n",
			lexer->tokens[i].end - lexer->tokens[i].start,
			lexer->tokens[i].end - lexer->tokens[i].start,
			&code[lexer->tokens[i].start],
			lexer->tokens[i].key_end - lexer->tokens[i].key_start,
			&code[lexer->tokens[i].key_start],
			lexer->tokens[i].lock_end - lexer->tokens[i].lock_start,
			&code[lexer->tokens[i].lock_start],
			lexer->tokens[i].type,
			lexer->tokens[i].subtype1,
			lexer->tokens[i].subtype2);
	}
}
#endif
