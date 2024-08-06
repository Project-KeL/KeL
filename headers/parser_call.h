#ifndef PARSER_CALL
#define PARSER_CALL

#include "allocator.h"
#include "parser_def.h"

int if_call_create_nodes(
	size_t* i,
	const MemoryChainLink* link_label_parameterized_scope,
	const Node* node_label_parameterized,
	Node** node_call_last,
	Parser* parser);

#endif
