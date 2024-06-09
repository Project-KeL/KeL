#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include "allocator.h"
#include "lexer.h"
#include "parser_def.h"

void initialize_parser(Parser* parser);
bool create_parser(
	const Lexer* lexer,
	MemoryArea* restrict memArea,
	Parser* parser);
void destroy_parser(Parser* parser);

#endif
