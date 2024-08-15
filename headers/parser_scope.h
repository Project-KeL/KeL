#ifndef PARSER_SCOPE
#define PARSER_SCOPE

#include "parser_def.h"

int if_scope_start_create_node(
	size_t i,
	Parser* parser);
bool parser_is_scope_start(const Node* node);
bool parser_is_valid_scope_start(const Node* node);
Node** parser_scope_start_get_scope_end(Node* node);
Node** parser_scope_start_get_PAL(Node* node);
int if_scope_end_create_node(
	size_t i,
	Parser* parser);
bool parser_is_scope_end(const Node* node);
bool parser_is_valid_scope_end(const Node* node);
void parser_scope_end_set_scope_start(
	Node* node,
	Node* scope_start);
const Node* parser_scope_end_get_scope_start(const Node* node);

#endif
