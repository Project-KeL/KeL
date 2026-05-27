#ifndef PARSER_UTILS_H
#define PARSER_UTILS_H

#include "lexer.h"
//
// instructions
bool parser_is_instruction_exit(const Token* token);
bool parser_is_instruction_end(const Token* token);
bool parser_is_instruction_INIT_equal(const Token* token);
bool parser_is_LSCOPE_start(const Token* token);
bool parser_is_LSCOPE_end(const Token* token);
bool parser_is_IMOD(const Token* token);
bool parser_is_OMOD(const Token* token);
// misc
bool parser_is_parenthesis(const Token* token);
bool parser_is_bracket(const Token* token);
bool parser_is_L_left_parenthesis(const Token* token);
bool parser_is_L_right_parenthesis(const Token* token);
bool parser_is_R_underscope(const Token* token);
bool parser_is_R_left_parenthesis(const Token* token);
bool parser_is_R_right_parenthesis(const Token* token);
bool parser_is_command(const Token* token);
bool parser_is_Q(const Token* token);
bool parser_is_operator_algebraic(const Token* token);
bool parser_is_operator_MUL(const Token* token);
bool parser_is_operator_ADD(const Token* token);
bool parser_is_operator_SUB(const Token* token);
bool parser_is_operator_DIV(const Token* token);
bool parser_is_operator_leveling(const Token* token);
bool parser_is_operator_modifier(const Token* token);
bool parser_is_PAL_comma(const Token* token);
bool parser_is_literal(const Token* token);
bool parser_is_literal_number(const Token* token);
bool parser_is_special(const Token* token);
bool parser_is_ID(const Token* token);
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
bool parser_is_code_token_LR_match(
	const char* code,
	const Token* token1,
	const Token* token2);
bool parser_is_code_token_match(
	const char* code,
	const Token* token1,
	const Token* token2);

#endif
