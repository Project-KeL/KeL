#ifndef PARSER_UTILS_H
#define PARSER_UTILS_H

#include "parser.h"

bool is_scope(
	long int i,
	const Lexer* restrict lexer);
long int get_j_scope_parent(
	long int j,
	Parser* parser);

#endif
