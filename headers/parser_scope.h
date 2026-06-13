#ifndef PARSER_SCOPE_H
#define PARSER_SCOPE_H

#include "allocator.h"
#include "parser.h"

bool if_LSCOPE_create_context(
	size_t* i,
	MemoryStack* stack_context,
	MemoryStack* stack_operator,
	Parser* parser);
bool if_LSCOPE_end_destroy_context(
	size_t* i,
	size_t* j,
	MemoryStack* stack_context,
	MemoryStack* stack_operator,
	Parser* parser);

#endif
