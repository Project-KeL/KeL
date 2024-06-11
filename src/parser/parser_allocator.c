#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "parser_allocator.h"

#define CHUNK 1

void parser_initialize_allocators(Parser* parser) {
	assert(parser != NULL);

	initialize_memory_chain(&parser->nodes);
	initialize_memory_chain(&parser->declarations);
	parser->error_allocator = true;
}

bool parser_create_allocators(Parser* parser) {
	assert(parser != NULL);
	// the first node is null
	if(create_memory_chain(
		CHUNK,
		sizeof(Node),
		&parser->nodes)
	== false)
		return parser->error_allocator = false;

	if(create_memory_chain(
		CHUNK,
		sizeof(Node),
		&parser->declarations)
	== false)
		return parser->error_allocator = false;

	return true;
}

void parser_destroy_allocators(Parser* parser) {
	assert(parser != NULL);

	destroy_memory_chain(&parser->declarations);
	destroy_memory_chain(&parser->nodes);
}

bool parser_allocator_node(Parser* parser) {
	assert(parser != NULL);

	return parser->error_allocator = memory_chain_reserve_data(
		CHUNK,
		&parser->nodes);
}

bool parser_allocator_declaration(Parser* parser) {
	assert(parser != NULL);

	return parser->error_allocator = memory_chain_reserve_data(
		CHUNK,
		&parser->declarations);
}

#undef CHUNK
