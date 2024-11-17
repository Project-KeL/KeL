#ifndef PARSER_SCOPE
#define PARSER_SCOPE

#include "parser_def.h"

int if_scope_start_create_node(
	size_t i,
	Parser* parser);
bool parser_is_scope_start(const Node* node);
void parser_scope_start_set_scope_end(
	Node* scope_start,
	Node* scope_end);
void parser_scope_start_set_PAL(
	Node* scope_start,
	Node* PAL);
const Node* parser_scope_start_get_scope_end(const Node* scope_start);
const Node* parser_scope_start_get_PAL(const Node* scope_start);
int if_scope_end_create_node(
	size_t i,
	Parser* parser);
bool parser_is_scope_end(const Node* node);
void parser_scope_end_set_scope_start(
	Node* scope_end,
	Node* scope_start);
Node* parser_scope_end_get_scope_start(const Node* scope_end);

#endif
