#ifndef PARSER_ALLOCATION
#define PARSER_ALLOCATION

#include <stdlib.h>
#include "parser_def.h"

void parser_initialize_allocators(Parser* parser);
bool parser_create_allocators(Parser* parser);
void parser_destroy_allocators(Parser* parser);
bool parser_allocator(Parser* parser);
const Node* parser_allocator_start(
	const Parser* parser,
	const MemoryChainLink** link);
void parser_allocator_link_next(
	const Node* node,
	const MemoryChainLink** link);

#endif
