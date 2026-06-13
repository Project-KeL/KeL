#ifndef PARSER_ALLOCATION_H
#define PARSER_ALLOCATION_H

#include <stdlib.h>
#include "parser.h"

void parser_initialize_allocator(Parser* parser);
bool parser_create_allocator_chunk(Parser* parser);
bool parser_create_allocator(Parser* parser);
bool parser_create_allocator_limit(
	size_t limit,
	Parser* parser);
bool parser_create_allocator_chunk(Parser* parser);
bool parser_allocator_shrink_append_null(Parser* parser);
void parser_destroy_allocator(Parser* parser);

#endif
