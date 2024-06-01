#ifndef PARSER_IDENTIFIER
#define PARSER_IDENTIFIER

#include "allocator.h"
#include "parser_def.h"

int if_identifier_create_nodes(
	size_t* i,
	size_t* j,
	MemoryArea* memArea,
	Parser* parser);

#endif
