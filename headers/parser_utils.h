#ifndef PARSER_UTILS_H
#define PARSER_UTILS_H

#include "lexer_def.h"
#include "parser_def.h"

bool parser_is_scope(
	long int i,
	const Lexer* restrict lexer);
long int parser_get_j_scope_start_from_end(
	long int j,
	Parser* parser);
bool parser_is_operator_leveling(const Token* restrict token);
bool parser_is_operator_modifier(const Token* restrict token);
bool parser_is_special(const Token* token);
bool parser_is_lock(const Token* token);

#endif
