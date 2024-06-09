#ifndef PARSER_IDENTIFIER
#define PARSER_IDENTIFIER

#include "allocator.h"
#include "parser_def.h"

int if_identification_create_nodes(
	size_t* i,
	MemoryArea* restrict memArea,
	Parser* parser);

#endif
