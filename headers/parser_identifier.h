#ifndef PARSER_IDENTIFIER
#define PARSER_IDENTIFIER

#include "allocator.h"
#include "parser_def.h"

NodeSubtypeIdentificationBitCommand parser_identification_token_subtype_TO_node_subtype_identification_bit_command(TokenSubtype subtype_token);
int if_identification_create_nodes(
	bool nodes_initialization,
	size_t* i,
	MemoryArea* restrict memArea,
	Node** node_identification,
	Parser* parser);
bool parser_is_identification(const Node* node);
bool parser_identification_is_declaration(const Node* node);
bool parser_identification_is_initialization(const Node* node);
bool parser_identification_is_label(const Node* node);
bool parser_identification_is_label_parameterized(const Node* node);

#endif
