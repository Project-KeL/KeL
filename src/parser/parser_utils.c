#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "lexer.h"
#include "parser_utils.h"

// TODO: add asserts on "impossible" tests (exemple, parser_is_command: test all the subtype cases)

bool parser_is_instruction_exit(const Token* token) {
	return (token->type == TokenType_NO
	     && token->subtype == TokenSubtype_NO)
	    || (token->type == TokenType_LSPE
		 && token->subtype == TokenSubtype_SEMICOLON);
}

bool parser_is_instruction_end(const Token *token) {
	return token->type == TokenType_LSPE
	    && token->subtype == TokenSubtype_SEMICOLON;
}

bool parser_is_LSCOPE_start(const Token* token) {
	return token->type == TokenType_LSCOPE
	    && token->subtype == TokenSubtype_NO;
}

bool parser_is_LSCOPE_end(const Token* token) {
	return token->type == TokenType_LSPE
	    && token->subtype == TokenSubtype_PERIOD;
}

bool parser_is_IMOD(const Token* token) {
	return token->type == TokenType_L
	    && token->subtype == TokenSubtype_IMOD;
}

bool parser_is_OMOD(const Token* token) {
	return token->type == TokenType_L
	    && token->subtype == TokenSubtype_OMOD;
}

bool parser_is_CALL(const Token* token) {
	// if `token` is the last token, then it must be a null one
	return parser_is_key(token)
	    && parser_is_L_left_parenthesis(token + 1);
}

bool parser_is_call_separator(const Token* token) {
	return token->type == TokenType_LSPE
	    && token->subtype == TokenSubtype_COMMA;
}

bool parser_is_parenthesis(const Token* token) {
	return token->subtype == TokenSubtype_LPARENTHESIS
	    || token->subtype == TokenSubtype_RPARENTHESIS;
}

bool parser_is_bracket(const Token* token) {
	return token->subtype == TokenSubtype_LBRACKET
	    || token->subtype == TokenSubtype_RBRACKET;
}

bool parser_is_L_left_parenthesis(const Token* token) {
	return token->type == TokenType_LSPE
	    && token->subtype == TokenSubtype_LPARENTHESIS;
}

bool parser_is_L_right_parenthesis(const Token* token) {
	return token->type == TokenType_LSPE
	    && token->subtype == TokenSubtype_RPARENTHESIS;
}

bool parser_is_R_underscope(const Token* token) {
	return token->type == TokenType_RSPE
	    && token->subtype == TokenSubtype_UNDERSCORE;
}

bool parser_is_R_left_parenthesis(const Token* token) {
	return token->type == TokenType_RSPE
	    && token->subtype == TokenSubtype_LPARENTHESIS;
}

bool parser_is_R_right_parenthesis(const Token* token) {
	return token->type == TokenType_RSPE
	    && token->subtype == TokenSubtype_RPARENTHESIS;
}

bool parser_is_command(const Token* token) {
	return token->type == TokenType_COM;
}

bool parser_is_Q(const Token* token) {
	return token->type == TokenType_Q;
}

bool parser_is_operator_ASSIGN(const Token* token) {
	return token->type == TokenType_LSPE
	    && token->subtype == TokenSubtype_EQUAL;
}

bool parser_is_operator_algebraic(const Token* token) {
	return token->type == TokenType_LSPE
	    && (token->subtype == TokenSubtype_ASTERISK
	     || token->subtype == TokenSubtype_PLUS
	     || token->subtype == TokenSubtype_MINUS
	     || token->subtype == TokenSubtype_DIVIDE
	     || token->subtype == TokenSubtype_EQUAL);
}

bool parser_is_operator_MUL(const Token* token) {
	return token->type == TokenType_LSPE
	    && token->subtype == TokenSubtype_ASTERISK;
}

bool parser_is_operator_ADD(const Token* token) {
	return token->type == TokenType_LSPE
	    && token->subtype == TokenSubtype_PLUS;
}

bool parser_is_operator_SUB(const Token* token) {
	return token->type == TokenType_LSPE
	    && token->subtype == TokenSubtype_MINUS;
}

bool parser_is_operator_DIV(const Token* token) {
	return token->type == TokenType_LSPE
	    && token->subtype == TokenSubtype_DIVIDE;
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

bool parser_is_RSCOPE(const Token* token) {
	return token->type == TokenType_RSCOPE
	    && token->subtype == TokenSubtype_NO;
}

bool parser_is_PAL_comma(const Token* token) {
	return token->type == TokenType_LSPE
	    && token->subtype == TokenSubtype_COMMA;
}

bool parser_is_literal(const Token* token) {
	return token->type == TokenType_LIT;
}

bool parser_is_literal_number(const Token* token) {
	return token->subtype == TokenSubtype_LIT_NUM;
}

bool parser_is_special(const Token* token) {
	return token->subtype >= TokenSubtype_EXCLAMATION_MARK
	    && token->subtype <= TokenSubtype_TILDE;
}

bool parser_is_ID(const Token* token) {
	return token->type == TokenType_ID;
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

bool parser_is_code_token_side_L_match(
const char* code,
const Token* token1,
const Token* token2) {
	if(token1 == NULL
	|| token2 == NULL
	|| token1->end - token1->start != token2->end - token2->start)
		return false;

	return strncmp(
		code + token1->start,
		code + token2->start,
		token1->end - token1->start) == 0;
}

bool parser_is_code_token_L_match(
const char* code,
const Token* token1,
const Token* token2) {
	if(token1 == NULL
	|| token2 == NULL
	|| token1->type != TokenType_L
	|| token2->type != TokenType_L)
		return false;

	return strncmp(
		code + token1->start,
		code + token2->start,
		token1->end - token1->start) == 0;
}

bool parser_is_code_token_R_match(
const char* code,
const Token* token1,
const Token* token2) {
	if(token1 == NULL
	|| token2 == NULL
	|| token1->type != TokenType_R
	|| token2->type != TokenType_R)
		return false;

	return strncmp(
		code + token1->start,
		code + token2->start,
		token1->end - token1->start) == 0;
}

bool parser_is_code_token_LR_match(
const char* code,
const Token* token1,
const Token* token2) {
	if(token1 == NULL
	|| token2 == NULL
	|| token1->type != TokenType_LR
	|| token2->type != TokenType_LR)
		return false;

	return parser_is_code_token_L_match(
		code,
		token1,
		token2)
	    && parser_is_code_token_R_match(
		code,
		token1,
		token2);
}

bool parser_is_code_token_match(
const char* code,
const Token* token1,
const Token* token2) {
	if(token1 == NULL
	|| token2 == NULL)
		return false;

	long int start1;
	long int start2;
	long int end1;
	long int end2;

	if(token1->end == token1->start) {
		start1 = token1->start;
		end1 = token1->start;
	} else {
		start1 = token1->start;
		end1 = token1->end;
	}

	if(token2->end == token2->start) {
		start2 = token2->start;
		end2 = token2->start;
	} else {
		start2 = token2->start;
		end2 = token2->end;
	}

	if(end1 - start1 != end2 - start2)
		return false;

	return strncmp(
		code + start1,
		code + start2,
		end1 - start1) == 0;
}
