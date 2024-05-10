#include <string.h>
#include "parser_error.h"
#include <stdio.h>

// scope key must match with a dot

bool parser_scan_errors(const Lexer* restrict lexer) {
	const char* code = lexer->source->content;
	size_t count_scope_nest = 0;

	for(long int i = 0;
	i < lexer->count;
	i += 1) {
		const Token* token = &lexer->tokens[i];

		if(token->type == TokenType_L) {
			if(strcmp(
				"scope",
				&code[token->L_start])
			<= 0)
				count_scope_nest += 1;
		} else if(token->subtype == TokenSubtype_PERIOD) {
			if(count_scope_nest == 0)
				return false;

			count_scope_nest -= 1;
		}
	}

	return count_scope_nest == 0;
}
