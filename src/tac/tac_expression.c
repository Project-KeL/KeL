#include <assert.h>
#include "tac_expression.h"
#include <stdio.h>

void tac_create_expression(
size_t start,
size_t end,
const STab* stab) {
	const Parser* parser = stab->parser;
	const Node* nodes = parser->nodes.base;

	for(
	size_t i = start;
	i < end;
	i += 1) {
		const Token* token = ((Token*) parser->lexer->tokens.base) + nodes[i].offset_token;
		printf("%.*s ", token->end - token->start, parser->lexer->source->content + token->start);
	}
	printf("\n");
}
