#ifndef PARSER_TYPE
#define PARSER_TYPE

#include "allocator.h"
#include "parser_def.h"

int if_type_create_nodes(
	size_t* i,
	MemoryArea* restrict memArea,
	NodeSubtypeIdentificationBitScoped* restrict bit_scoped,
	Parser* parser); 

#endif
