#ifndef PARSER_TYPE
#define PARSER_TYPE

#include "allocator.h"
#include "parser_def.h"

int if_type_create_nodes(
	size_t* i,
	MemoryArea* restrict memArea,
	NodeSubtypeIntroductionBitScoped* restrict bit_scoped,
	Parser* parser); 
bool parser_is_type(const Node* node);
void parser_type_set_tail(
	Node* type,
	Node* tail);
Node* parser_type_get_tail(const Node* type);

#endif
