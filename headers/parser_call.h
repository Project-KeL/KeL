#ifndef PARSER_CALL
#define PARSER_CALL

#include "allocator.h"
#include "parser_def.h"

int if_call_create_nodes(
	size_t* i,
	const Node* node_parameterized_label,
	const MemoryChainLink* link_parameterized_label_scope,
	Node** node_call_last,
	Parser* parser);

#endif
