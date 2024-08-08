#ifndef PARSER_IDENTIFIER
#define PARSER_IDENTIFIER

#include "allocator.h"
#include "parser_def.h"

NodeSubtypeIntroductionBitCommand parser_identifier_token_subtype_TO_node_subtype_introduction_bit_command(TokenSubtype subtype_token);
int if_identifier_create_nodes(
	bool nodes_initialization,
	size_t* i,
	MemoryArea* restrict memArea,
	Node** node_identifier,
	Parser* parser);
bool parser_is_identifier(const Node* node);
bool parser_identifier_is_declaration(const Node* node);
bool parser_identifier_is_initialization(const Node* node);
bool parser_identifier_is_label(const Node* node);
bool parser_identifier_is_PAL(const Node* node);

#endif
