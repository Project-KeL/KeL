#ifndef PARSER_NODE
#define PARSER_NODE

#include <stddef.h>
#include "alloca.h"
#include "parser.h"

void parser_create_leaf(
	NodeType type,
	size_t token,
	size_t* j,
	MemoryStack* stack_operator,
	Parser* parser);
void parser_create_operator(
	NodeType type,
	uint32_t arity,
	size_t token,
	size_t* j,
	MemoryStack* stack_operator,
	Parser* parser);

#endif
