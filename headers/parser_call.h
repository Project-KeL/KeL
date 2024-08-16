#ifndef PARSER_CALL
#define PARSER_CALL

#include "allocator.h"
#include "parser_def.h"

int if_call_create_nodes(
	size_t* i,
	const MemoryChainLink* link_PAL,
	const Node* node_PAL,
	Node** node_call_last,
	Parser* parser);
bool parser_is_call(const Node* node);
bool parser_is_valid_call(const Node* node);
bool parser_call_is_time_compile(const Node* node);
bool parser_call_is_time_binary(const Node* node);
bool parser_call_is_time_run(const Node* node);
bool parser_call_is_return(const Node* node);
bool parser_call_is_return_deduce(const Node* node);
void parser_call_set_PAL(
	Node* node,
	Node* PAL);
const Node* parser_call_get_PAL(const Node* node);
bool parser_call_is_child(const Node* node);
bool parser_call_is_valid_child(const Node* child);

#endif
