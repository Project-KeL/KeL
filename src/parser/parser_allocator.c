#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "parser_allocator.h"

#define CHUNK 1

void parser_initialize_allocators(Parser* parser) {
	assert(parser != NULL);

	initialize_memory_chain(&parser->nodes);
	initialize_memory_chain(&parser->identifiers);
	initialize_memory_chain(&parser->identifiers_parameterized);
	parser->error_allocator = true;
}

bool parser_create_allocators(Parser* parser) {
	assert(parser != NULL);

	if(create_memory_chain(
		CHUNK,
		sizeof(Node),
		&parser->nodes)
	== false)
		return parser->error_allocator = false;
	// null token implicit (calloc)
	if(create_memory_chain(
		CHUNK,
		sizeof(Node*),
		&parser->identifiers)
	== false)
		return parser->error_allocator = false;

	if(create_memory_chain(
		CHUNK,
		sizeof(Node*),
		&parser->identifiers_parameterized)
	== false)
		return parser->error_allocator = false;

	return true;
}

void parser_destroy_allocators(Parser* parser) {
	assert(parser != NULL);

	destroy_memory_chain(&parser->identifiers_parameterized);
	destroy_memory_chain(&parser->identifiers);
	destroy_memory_chain(&parser->nodes);
}

bool parser_allocator_node(Parser* parser) {
	assert(parser != NULL);

	return parser->error_allocator = memory_chain_reserve_data(
		CHUNK,
		&parser->nodes);
}

bool parser_allocator_identifier(Parser* parser) {
	assert(parser != NULL);

	return parser->error_allocator = memory_chain_reserve_data(
		CHUNK,
		&parser->identifiers);
}

bool parser_allocator_identifier_parameterized(Parser* parser) {
	assert(parser != NULL);

	return parser->error_allocator = memory_chain_reserve_data(
		CHUNK,
		&parser->identifiers_parameterized);
}
#undef CHUNK
