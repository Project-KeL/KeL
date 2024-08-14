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
bool parser_is_valid_call(const Node* node);
bool parser_call_is_time_compile(const Node* node);
bool parser_call_is_time_binary(const Node* node);
bool parser_call_is_time_run(const Node* node);
bool parser_call_is_return(const Node* node);
bool parser_call_is_return_deduce(const Node* node);
Node* parser_call_get_PAL(Node* node);

#endif
