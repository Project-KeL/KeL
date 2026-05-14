#ifndef PARSER_ALLOCATION
#define PARSER_ALLOCATION

#include <stdlib.h>
#include "parser.h"

void parser_initialize_allocator(Parser* parser);
bool parser_create_allocator_chunk(Parser* parser);
bool parser_create_allocator(Parser* parser);
bool parser_create_allocator_limit(
	size_t limit,
	Parser* parser);
bool parser_create_allocator_chunk(Parser* parser);
bool parser_allocator_shrink(Parser* parser);
void parser_destroy_allocator(Parser* parser);

#endif
