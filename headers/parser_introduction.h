#ifndef PARSER_IDENTIFIER
#define PARSER_IDENTIFIER

#include "allocator.h"
#include "parser_def.h"

NodeSubtypeIntroductionBitCommand parser_identifier_token_subtype_TO_node_subtype_introduction_bit_command(TokenSubtype subtype_token);
int if_introduction_create_nodes(
	bool nodes_initialization,
	size_t* i,
	MemoryArea* restrict memArea,
	MemoryChainLink** link_introduction,
	Node** node_introduction,
	Parser* parser);
bool parser_is_introduction(const Node* node);
bool parser_introduction_is_declaration(const Node* node);
bool parser_introduction_is_initialization(const Node* node);
bool parser_introduction_is_label(const Node* node);
bool parser_introduction_is_PAL(const Node* node);
bool parser_introduction_is_command_hash(const Node* node);
bool parser_introduction_is_command_at(const Node* node);
bool parser_introduction_is_command_exclamation_mark(const Node* node);
void parser_introduction_set_type(
	Node* introduction,
	Node* type);
void parser_introduction_set_initialization(
	Node* introduction,
	Node* initialization);
Node* parser_introduction_get_type(const Node* introduction);
Node* parser_introduction_get_initialization(const Node* introduction);

#endif
