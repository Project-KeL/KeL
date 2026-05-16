#ifndef PARSER_TYPE
#define PARSER_TYPE

#include <stddef.h>
#include "allocator.h"
#include "parser.h"

bool if_TYPE_create_operator(
	size_t* i,
	size_t* j,
	MemoryStack* stack_operator,
	Parser* parser) ;

#endif
