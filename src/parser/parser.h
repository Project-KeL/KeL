#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef struct {
} Parser;

bool parse(
	Lexer* restrict tokens,
	Parser* restrict parser);

#endif
