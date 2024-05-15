#ifndef NDEBUG
#include <inttypes.h>
#include <stdio.h>
#include "debug.h"

void debug_print_tokens(const Lexer* lexer) {
	const char* code = lexer->source->content;

	for(long int i = 0;
	i < lexer->count - 1;
	i += 1) {
		const Token* token = &lexer->tokens[i];
		const long int token_L_length = token->L_end - token->L_start;
		const long int token_R_length = token->R_end - token->R_start;
		long int subtype = token->subtype;

		if(token->type == TokenType_QL)
			subtype >>= SHIFT_TOKEN_SUBTYPE_QL;
		else if(token->type == TokenType_QR)
			subtype >>= SHIFT_TOKEN_SUBTYPE_QR;

		printf(
			"%d, %d: (\"%.*s\", \"%.*s\") {%" PRIu32 ", %" PRIu32 "}\n",
			token_L_length,
			token_R_length,
			token_L_length,
			&code[token->L_start],
			token_R_length,
			&code[token->R_start],
			token->type,
			subtype);
	}

	printf(
		"\nNumber of tokens: %d",
		lexer->count - 1);
}

void debug_print_nodes(const Parser* parser) {
	for(long int j = 0;
	j < parser->count - 1;
	j += 1) {
		const Node* node = &parser->nodes[j];
		printf(
			"%" PRIu64 "\n",
			node->type >> 48);
	}

	printf(
		"\nNumber of nodes: %d",
		parser->count);
}

#endif
