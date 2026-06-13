#ifndef PARSER_MODULE_H
#define PARSER_MODULE_H

#include "allocator.h"
#include "parser.h"

bool if_MOD_create_operator(
	size_t* i,
	size_t* j,
	MemoryStack* stack_operator,
	Parser* parser);

#endif
