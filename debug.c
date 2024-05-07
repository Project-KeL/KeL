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
		printf(
			"%d, %d: (\"%.*s\", \"%.*s\") {%" PRIu32 ", %" PRIu32 "}\n",
			token->key_end - token->key_start,
			token->lock_end - token->lock_start,
			token->key_end - token->key_start,
			&code[token->key_start],
			token->lock_end - token->lock_start,
			&code[token->lock_start],
			token->type,
			token->subtype);
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
		parser->count - 1);
}

#endif
