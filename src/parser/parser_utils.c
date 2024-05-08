#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "lexer_utils.h"
#include "parser_utils.h"

// look at the commit 147b4b12 to get back the string to uint64_t converter

bool is_scope(
long int i,
const Lexer* restrict lexer) {
	const Token* token = &lexer->tokens[i];

	if(token->type != TokenType_KEY
	|| strncmp(
		"scope",
		&lexer->source->content[token->key_start],
		token->key_end - token->key_start)
	!= 0)
		return false;

	return true;
}

long int get_j_scope_parent(
long int j,
Parser* parser) {
	NodeType type;

	do {
		j -= 1;
		type = parser->nodes[j].type;
	} while(type != NodeType_SCOPE_START
	     && j > 0);

	return j;
}
