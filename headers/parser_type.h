#ifndef PARSER_TYPE
#define PARSER_TYPE

#include "allocator.h"
#include "parser_def.h"

int if_type_create_nodes(
	size_t* i,
	MemoryArea* restrict memArea,
	NodeSubtypeIntroductionBitScoped* restrict bit_scoped,
	Parser* parser); 
bool parser_is_valid_type(const Node* node);
void parser_type_set_next(
	Node* node,
	Node* next);
const Node* parser_type_get_next(const Node* node);

#endif
