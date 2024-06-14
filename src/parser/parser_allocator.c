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

bool parser_allocator(Parser* parser) {
	assert(parser != NULL);

	return parser->error_allocator = memory_chain_reserve_data(
		CHUNK,
		&parser->nodes);
}

const Node* parser_allocator_start(
const Parser* parser,
const MemoryChainLink** link) {
	assert(parser != NULL);
	assert(link != NULL);

	Node* node;

	if(parser->declarations.first->memArea.count > 1) {
		*link = parser->declarations.first;
		node = (Node*) (*link)->memArea.addr + 1;
	} else {
		*link = parser->declarations.first->next;
		node = (Node*) (*link)->memArea.addr;
	}

	return node;
}

void parser_allocator_link_next(
const Node* node,
const MemoryChainLink** link) {
	if(node == (Node*) (*link)->memArea.addr + (*link)->memArea.count - 1)	
		*link = (*link)->next;
}

#undef CHUNK
