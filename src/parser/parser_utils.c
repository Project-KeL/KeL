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

bool parser_is_command(const Token* token) {
	return token->type == TokenType_COMMAND;
}

bool parser_is_qualifier(const Token* token) {
	return token->type == TokenType_QL
	    || token->type == TokenType_QR
	    || token->type == TokenType_QLR;
}

bool parser_is_operator_leveling(const Token* token) {
	return token->subtype == TokenSubtype_AMPERSAND
	    || token->subtype == TokenSubtype_MINUS
	    || token->subtype == TokenSubtype_PIPE
	    || token->subtype == TokenSubtype_PLUS;
}

bool parser_is_operator_modifier(const Token* token) {
	return parser_is_operator_leveling(token)
	    || parser_is_bracket(token);
}

bool parser_is_scope_L(const Token* token) {
	return token->type == TokenType_L
	    && token->subtype == TokenSubtype_SCOPE;
}

bool parser_is_scope_R(const Token* token) {
	return token->type == TokenType_R
	    && token->subtype == TokenSubtype_SCOPE;
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

bool parser_is_code_token_match(
const char* code,
const Token* token1,
const Token* token2) {
	long int start1;
	long int start2;
	long int end1;
	long int end2;

	if(token1->L_end - token1->L_start == 0) {
		start1 = token1->L_start;
		end1 = token1->L_start;
	} else {
		start1 = token1->R_start;
		end1 = token1->R_end;
	}

	if(token2->L_end - token2->L_start == 0) {
		start2 = token2->L_start;
		end2 = token2->L_start;
	} else {
		start2 = token2->R_start;
		end2 = token2->R_end;
	}

	if(end1 - start1 != end2 - start2)
		return false;

	return strncmp(
		code + start1,
		code + start2,
		end1 - start1) == 0;
}
