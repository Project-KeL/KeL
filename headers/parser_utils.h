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
bool parser_is_qualifier(const Token* token);
bool parser_is_operator_leveling(const Token* restrict token);
bool parser_is_operator_modifier(const Token* restrict token);
bool parser_is_scope(
	long int i,
	const Lexer* restrict lexer);
Node* parser_get_scope_from_period(Parser* parser);
bool parser_is_special(const Token* token);
bool parser_is_key(const Token* token);
bool parser_is_lock(const Token* token);

#endif
