#include <assert.h>
#include <stddef.h>
#include "parser_allocator.h"

#define CHUNK_SYMBOLS 1
#define CHUNK_NODES 1

#define CHUNK 4096

static void create_node_null(Node* token) {
	*token = (Node) {
		.type = NodeType_NO,
		.arity = 0,
		.token = 0};
}

void parser_initialize_allocator(Parser* parser) {
	assert(parser != NULL);

	initialize_memory_area(&parser->nodes);
}

bool parser_create_allocator_limit(
size_t limit,
Parser* parser) {
	if(create_memory_area(
		limit,
		sizeof(Node),
		&parser->nodes)
	== false)
		return false;

	create_node_null((Node*) parser->nodes.base);
	return true;
}

bool parser_create_allocator_chunk(Parser* parser) {
	return parser_create_allocator_limit(
		CHUNK,
		parser);
}

bool parser_allocator(
size_t minimum,
Parser* parser) {
	if(parser->nodes.count <= minimum) {
		if(memory_area_realloc(
			(minimum / CHUNK + 1) * CHUNK,
			&parser->nodes)
		== false) {
			assert(false); // the `parser_create_allocator_limit` must be used
			return false;
		}
	}

	return true;
}

bool parser_allocator_shrink_append_null(Parser* parser) {
	assert(parser->nodes.count <= (size_t) parser->lexer->source->length);

	const bool error = memory_area_realloc(
		parser->nodes.count + 1, // null token
		&parser->nodes);
	// sentinel
	create_node_null((Node*) parser->nodes.base + parser->nodes.count - 1);
	return error;
}

void parser_destroy_allocator(Parser* parser) {
	if(parser == NULL)
		return;

	destroy_memory_area(&parser->nodes);
}

#undef CHUNK
