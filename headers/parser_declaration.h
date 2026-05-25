#ifndef PARSER_DECLARATION
#define PARSER_DECLARATION

#include "allocator.h"
#include "parser.h"

bool if_DECL_create_operator(
	size_t* i,
	size_t* j,
	MemoryStack* stack_contexts,
	MemoryStack* stack_operators,
	MemoryStack* srack_buffer,
	Parser* parser);

#endif
