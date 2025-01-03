#ifndef PARSER_UTILS_H
#define PARSER_UTILS_H

#include "lexer_def.h"
#include "parser_def.h"

bool parser_is_parenthesis(const Token* token);
bool parser_is_bracket(const Token* token);
bool parser_is_L_left_parenthesis(const Token* token);
bool parser_is_L_right_parenthesis(const Token* token);
bool parser_is_R_grave_accent(const Token* token);
bool parser_is_R_left_parenthesis(const Token* token);
bool parser_is_R_right_parenthesis(const Token* token);
bool parser_is_command(const Token* token);
bool parser_is_qualifier(const Token* token);
bool parser_is_operator_leveling(const Token* token);
bool parser_is_operator_modifier(const Token* token);
bool parser_is_scope_L(const Token* token);
bool parser_is_scope_R(const Token* token);
bool parser_is_special(const Token* token);
bool parser_is_key(const Token* token);
bool parser_is_lock(const Token* token);
bool parser_is_code_token_side_L_match(
	const char* code,
	const Token* token1,
	const Token* token2);
bool parser_is_code_token_side_R_match(
	const char* code,
	const Token* token1,
	const Token* token2);
bool parser_is_code_token_L_match(
	const char* code,
	const Token* token1,
	const Token* token2);
bool parser_is_code_token_R_match(
	const char* code,
	const Token* token1,
	const Token* token2);
bool parser_is_code_token_LR_match(
	const char* code,
	const Token* token1,
	const Token* token2);
bool parser_is_code_token_match(
	const char* code,
	const Token* token1,
	const Token* token2);

#endif
