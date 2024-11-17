#ifndef PARSER_MODULE
#define PARSER_MODULE

#include "parser_def.h"

int if_module_create_nodes(
	size_t* i,
	Node** node_module_last,
	Parser* parser);
void parser_module_set_tail(
	Node* module,
	Node* tail);
const Node* parser_module_get_tail(const Node* module);

#endif
