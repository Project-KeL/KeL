#include <string.h>
#include "parser_error.h"
#include <stdio.h>

/*
 * 1. a scope must match with a dot
 * 2. pattern of a lock for a key declaration or an initialization
 */
/*
static bool scan_type(
const Lexer* restrict lexer,
long int* i) {
	const Token* tokens = lexer->tokens;
	long int buffer_i = *i;

	while(true) {
		while(is_operator_pointer(tokens[buffer_i])) {
			buffer += 1;
		}

		if(tokens[buffer_i].type == TokenType_LOCK) {
		}
	}
}
*/
bool parser_scan_errors(const Lexer* restrict lexer) {
	const char* code = lexer->source->content;
	size_t count_scope_nest = 0;

	for(long int i = 0;
	i < lexer->count;
	i += 1) {
		const Token* tokens = lexer->tokens;

		if(tokens[i].type == TokenType_L) {
			if(strncmp(
				"scope",
				&code[tokens[i].L_start],
				tokens[i].L_end - tokens[i].L_start)
			== 0)
				count_scope_nest += 1;
		} else if(tokens[i].subtype == TokenSubtype_PERIOD) {
			if(count_scope_nest == 0)
				return false;

			count_scope_nest -= 1;
		} /* else if(tokens[i].type == TokenType_COMMAND
			   && tokens[i + 1].type == TokenType_L) {
			long int buffer_i = i + 2;

			if(tokens[buffer_i].type == TokenType_L) {

			}
			while(true) {
				if(lexer_is_operator_leveling(tokens[buffer_i])) {
					do {
						buffer_i += 1;
					while(lexer_is_operator_pointer(&tokens[buffer_i])
					   && tokens[buffer_i].subtype != TokenSubtype_COLON);
				} else

				if(tokens[buffer_i].type != TokenType_R)
					return false;
			}
		} */
	}

	return count_scope_nest == 0;
}
