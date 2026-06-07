#ifndef PARSER_EXPRESSION
#define PARSER_EXPRESSION

#include <stddef.h>
#include "allocator.h"
#include "parser.h"


bool if_EXP_create_operator(
	size_t* i,
	size_t* j,
	MemoryStack* stack_context,
	MemoryStack* stack_operator,
	MemoryStack* stack_buffer,
	Parser* parser);

#endif
