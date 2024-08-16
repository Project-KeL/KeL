#ifndef PARSER_MODULE
#define PARSER_MODULE

#include "parser_def.h"

int if_module_create_nodes(
	size_t* i,
	Node** node_module_last,
	Parser* parser);
bool parser_is_module(const Node* node);
bool parser_is_valid_module(const Node* node);
void parser_module_set_next(
	Node* node,
	Node* next);
const Node* parser_module_get_next(const Node* node);

#endif
