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

#endif
