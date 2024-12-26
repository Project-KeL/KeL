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
void parser_node_set_tail(
	Node* node,
	Node* tail);
Node* parser_node_get_tail(const Node* node);

#endif
