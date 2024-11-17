#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "parser_allocator.h"

#define CHUNK 1

void parser_initialize_allocators(Parser* parser) {
	assert(parser != NULL);

	initialize_memory_chain(&parser->nodes);
	initialize_memory_chain(&parser->file_nodes);
}

bool parser_create_allocators(Parser* parser) {
	assert(parser != NULL);
	// the first node is null
	if(create_memory_chain(
		CHUNK,
		sizeof(Node),
		&parser->nodes)
	== false)
		return false;

	if(create_memory_chain(
		CHUNK,
		sizeof(Node),
		&parser->file_nodes)
	== false)
		return false;
#ifndef NDEBUG
	parser->count_nodes = 0;
	parser->count_file_nodes = 0;
#endif
	return true;
}

void parser_destroy_allocators(Parser* parser) {
	assert(parser != NULL);

	destroy_memory_chain(&parser->file_nodes);
	destroy_memory_chain(&parser->nodes);
}

bool parser_allocator(Parser* parser) {
	assert(parser != NULL);
#ifndef NDEBUG
	parser->count_nodes += 1;
#endif
	return memory_chain_reserve_data(
		CHUNK,
		&parser->nodes);
}

void parser_allocator_next_link(
const MemoryChainLink** link,
const Node* node) {
	assert(link != NULL);
	assert(node != NULL);

	if(node == (Node*) (*link)->memArea.addr + (*link)->memArea.count - 1)	
		*link = (*link)->next;
}

bool parser_allocator_next(
const Parser* parser,
const MemoryChainLink** link,
const Node** node) {
	assert(parser != NULL);
	assert(link != NULL);
	assert(node != NULL);

	if(*node == (Node*) (*link)->memArea.addr + (*link)->memArea.count - 1) {
		if((*link)->next == NULL)
			return false;

		*link = (*link)->next;
		*node = (Node*) (*link)->memArea.addr;
	} else
		*node += 1;

	return true;
}

const Node* parser_allocator_start_node(
const Parser* parser,
const MemoryChainLink** link) {
	assert(parser != NULL);
	assert(link != NULL);

	Node* node;

	if(parser->nodes.first->memArea.count > 1) {
		*link = parser->nodes.first;
		node = (Node*) (*link)->memArea.addr + 1;
	} else if(parser->nodes.first->next != NULL) {
		*link = parser->nodes.first->next;
		node = (Node*) (*link)->memArea.addr;
	} else
		return NULL;

	return node;
}

[[deprecated]] bool parser_allocator_continue_node(
const Parser* parser,
const Node* node) {
	assert(parser != NULL);
	assert(node != NULL);

	return node != (Node*) parser->nodes.last->memArea.addr + parser->nodes.last->memArea.count;
}

const Node* parser_allocator_start_file_node(
const Parser* parser,
const MemoryChainLink** link) {
	assert(parser != NULL);
	assert(link != NULL);

	Node* node;

	if(parser->nodes.first->memArea.count > 1) {
		*link = parser->file_nodes.first;
		node = (Node*) (*link)->memArea.addr + 1;
	} else if(parser->file_nodes.first->next != NULL) {
		*link = parser->file_nodes.first->next;
		node = (Node*) (*link)->memArea.addr;
	} else
		return NULL;

	return node;
}

[[deprecated]] bool parser_allocator_continue_file_node(
const Parser* parser,
const Node* node) {
	assert(parser != NULL);
	assert(node != NULL);

	return node != (Node*) parser->file_nodes.last->memArea.addr + parser->file_nodes.last->memArea.count;
}

void parser_allocator_node_previous(
MemoryChainLink* restrict* link,
Node** node) {
	assert(link != NULL);
	assert((*link)->previous != NULL
		|| (Node*) (*link)->memArea.addr < *node);

	if(*node == (*link)->memArea.addr) {
		*link = (*link)->previous;
		*node = (Node*) (*link)->memArea.addr + (*link)->memArea.count - 1;
	} else
		*node -= 1;
}

#undef CHUNK
