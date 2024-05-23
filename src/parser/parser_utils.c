#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "lexer_utils.h"
#include "parser_utils.h"

// look at the commit 147b4b12 to get back the string to uint64_t converter
//

bool parser_is_parenthesis(
const char* code,
const Token* token) {
	return lexer_is_parenthesis(code[token->L_start]);
}

bool parser_is_scope(
long int i,
const Lexer* restrict lexer) {
	const Token* token = &lexer->tokens[i];

	if(token->type != TokenType_L
	|| strncmp(
		"scope",
		&lexer->source->content[token->L_start],
		token->L_end - token->L_start)
	!= 0)
		return false;

	return true;
}

long int parser_get_j_scope_start_from_end(
long int j,
Parser* parser) {
	const Node* nodes = parser->nodes;
	long int count_scope_nest = 1;
	// if we are at a period, we must ignore it to get the start of the scope
	// if we are at the start of a scope, we get the previous one.
	do {
		j -= 1;

		if(nodes[j].type == NodeType_SCOPE_END)
			count_scope_nest += 1;

		if(nodes[j].type == NodeType_SCOPE_START)
			count_scope_nest -= 1;
	} while(count_scope_nest != 0
	     || parser->nodes[j].type != NodeType_SCOPE_START);

	return j;
}

bool parser_is_operator_leveling(const Token* restrict token) {
	return token->subtype == TokenSubtype_AMPERSAND
	    || token->subtype == TokenSubtype_MINUS
	    || token->subtype == TokenSubtype_PIPE
	    || token->subtype == TokenSubtype_PLUS;
}

bool parser_is_operator_modifier(const Token* restrict token) {
	return parser_is_operator_leveling(token)
	    || token->subtype == TokenSubtype_LBRACKET
	    || token->subtype == TokenSubtype_RBRACKET;
}

bool parser_is_special(const Token* token) {
	return token->subtype >= TokenSubtype_EXCLAMATION_MARK
	    && token->subtype <= TokenSubtype_TILDE;
}

bool parser_is_lock(const Token* token) {
	return token->type == TokenType_R
	    && token->subtype == TokenSubtype_NO;
}
