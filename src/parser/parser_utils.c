#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "lexer_utils.h"
#include "parser_utils.h"

// look at the commit 147b4b12 to get back the string to uint64_t converter

bool parser_is_parenthesis(const Token* token) {
	return token->subtype == TokenSubtype_LPARENTHESIS
	    || token->subtype == TokenSubtype_RPARENTHESIS;
}

bool parser_is_bracket(const Token* token) {
	return token->subtype == TokenSubtype_LBRACKET
	    || token->subtype == TokenSubtype_RBRACKET;
}

bool parser_is_L_left_parenthesis(const Token* token) {
	return token->type == TokenType_L
	    && token->subtype == TokenSubtype_LPARENTHESIS;
}

bool parser_is_L_right_parenthesis(const Token* token) {
	return token->type == TokenType_L
	    && token->subtype == TokenSubtype_RPARENTHESIS;
}

bool parser_is_R_grave_accent(const Token* token) {
	return token->type == TokenType_R
	    && token->subtype == TokenSubtype_GRAVE_ACCENT;
}

bool parser_is_R_left_parenthesis(const Token* token) {
	return token->type == TokenType_R
	    && token->subtype == TokenSubtype_LPARENTHESIS;
}

bool parser_is_R_right_parenthesis(const Token* token) {
	return token->type == TokenType_R
	    && token->subtype == TokenSubtype_RPARENTHESIS;
}

bool parser_is_qualifier(const Token* token) {
	return token->type == TokenType_QL
	    || token->type == TokenType_QR
	    || token->type == TokenType_QLR;
}

bool parser_is_operator_leveling(const Token* restrict token) {
	return token->subtype == TokenSubtype_AMPERSAND
	    || token->subtype == TokenSubtype_MINUS
	    || token->subtype == TokenSubtype_PIPE
	    || token->subtype == TokenSubtype_PLUS;
}

bool parser_is_operator_modifier(const Token* restrict token) {
	return parser_is_operator_leveling(token)
	    || parser_is_bracket(token);
}

bool parser_is_scope_L(const Token* restrict token) {
	return token->type == TokenType_L
	    && token->subtype == TokenSubtype_SCOPE;
}

bool parser_is_scope_R(const Token* restrict token) {
	return token->type == TokenType_R
	    && token->subtype == TokenSubtype_SCOPE;
}

Node* parser_get_scope_from_period(Parser* parser) {
	Node* node = (Node*) parser->nodes.top;
	MemoryChainLink* link = parser->nodes.last;
	long int count_scope_nest = 1;
	uint64_t count = 0;

	do {
		// get the last node in the previous memory area
		if(node == link->memArea.addr) {
			link = link->previous;
			node = (Node*) link->memArea.addr + link->memArea.count - 1;
		} else
			node -= 1;

		if(!node->is_child) {
			switch(node->type) {
			case NodeType_SCOPE_END: count_scope_nest += 1; break;
			case NodeType_SCOPE_START: count_scope_nest -= 1; break;
			}
		}

		count += 1;
	} while(count_scope_nest != 0
	     || node->type != NodeType_SCOPE_START);

	node->value = count;
	return node;
}

bool parser_is_special(const Token* token) {
	return token->subtype >= TokenSubtype_EXCLAMATION_MARK
	    && token->subtype <= TokenSubtype_TILDE;
}

bool parser_is_key(const Token* token) {
	return (token->type == TokenType_L
	     || token->type == TokenType_PL)
	    && token->subtype == TokenSubtype_NO;
}

bool parser_is_lock(const Token* token) {
	// miss the qualifiers for the moment
	return token->type == TokenType_R
	    && token->subtype == TokenSubtype_NO;
}
