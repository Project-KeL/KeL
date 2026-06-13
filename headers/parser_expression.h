#ifndef PARSER_EXPRESSION_H
#define PARSER_EXPRESSION_H

#include <stddef.h>
#include "allocator.h"
#include "parser.h"


bool if_EXP_create_operator(
	size_t* i,
	size_t* j,
	MemoryStack* stack_operator,
	MemoryStack* stack_buffer,
	Parser* parser);

#endif
