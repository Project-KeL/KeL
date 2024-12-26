#include <assert.h>
#include <string.h>
#include "parser.h"
#include "parser_allocator.h"
#include "parser_call.h"
#include "parser_error.h"
#include "parser_introduction.h"
#include "parser_module.h"
#include "parser_scope.h"
#include "parser_utils.h"
#include <stdio.h>

static int error = 0;

static int set_error(int value) {
	if(error == -1)
		return -1;

	error = value;
	return value;
}

void initialize_parser(Parser* parser) {
	parser->lexer = NULL;
	parser_initialize_allocators(parser);
}

static bool parse_scope_file(
MemoryArea* restrict memArea,
Parser* parser) {
	size_t i = 0;
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;
	// the counter is triggered when the first initialized parameterized label is encountered
	size_t count_scope_nest = 0;
	// switch to insert declarations in the right place
	const MemoryChain buffer_memChain = parser->nodes;
	parser->nodes = parser->file_nodes;

	while(i < parser->lexer->tokens.count - 1) {
		MemoryChainLink* link_introduction = NULL;
		Node* node_introduction = NULL;

		if(parser_is_scope_L(tokens + i)) {
			if(count_scope_nest > 0)
				count_scope_nest += 1;

			i += 1;
		} else if(tokens[i].subtype == TokenSubtype_PERIOD) {
			if(count_scope_nest > 0)
				count_scope_nest -= 1;

			i += 1;
		}

		if(count_scope_nest == 0
		&& set_error(
			if_introduction_create_nodes(
				false,
				&i,
				memArea,
				&link_introduction,
				&node_introduction,
				parser))
		== 1) {
#ifndef NDEBUG
			parser->count_file_nodes += 1;
#endif
			if(parser_introduction_is_initialization(node_introduction)
			&& parser_introduction_is_PAL(node_introduction)) {
				count_scope_nest += 1;
			}
		} else {
			i += 1;
		}	

		if(error == -1)
			return false;
	}

	parser->file_nodes = parser->nodes;
	parser->nodes = buffer_memChain;
	return true;
}

bool create_parser(
const Lexer* lexer,
MemoryArea* restrict memArea,
Parser* parser) {
	assert(parser != NULL);
	assert(lexer != NULL);
	assert(memArea != NULL);

	parser->lexer = lexer;
	const Token* tokens = (const Token*) lexer->tokens.addr;
	size_t i = 1;
	size_t count_scope_nest = 0;
	// `node_previous` is the last node not being a child
	Node* node_previous = parser->nodes.top;
	MemoryChainLink* link_previous = NULL;
	// these variables start at the introduction of the current PAL
	MemoryChainLink* link_PAL_current = NULL;
	Node* node_PAL_current = NULL;

	if(!parser_scan_errors(lexer))
		return false;

	if(!parser_create_allocators(parser))
		return false;

	if(parse_scope_file(
		memArea,
		parser)
	== false)
		goto DESTROY;

	while(i < lexer->tokens.count - 1) {
		// create nodes
		if(parser_is_scope_L(tokens + i)) {
			if(if_scope_start_create_node(
				i,
				parser)
			== -1)
				goto DESTROY;

			count_scope_nest += 1;
			i += 1;

			if(node_previous != NULL
			&& parser_is_introduction(node_previous)
			&& parser_introduction_is_PAL(node_previous)) {
				// initialization with a scope case
				parser_introduction_set_initialization(
					node_previous,
					(Node*) parser->nodes.top);

				if(parser_introduction_is_initialization(node_previous))
					parser_scope_start_set_PAL(
						(Node*) parser->nodes.top,
						node_previous);
			}

			while(set_error(
				if_scope_start_create_node(
					i,
					parser))
			== 1) {
				count_scope_nest += 1;
				i += 1;
			}

			node_previous = (Node*) parser->nodes.top;
			// a scope can be empty
			if(tokens[i].subtype != TokenSubtype_PERIOD)
				continue; // no semicolon required
		} else if(set_error(
			if_introduction_create_nodes(
				true,
				&i,
				memArea,
				&link_previous,
				&node_previous,
				parser))
		== 1) {
			if(parser_introduction_is_PAL(node_previous)
			&& parser_introduction_is_initialization(node_previous)) {
				// no nested parameterized label
				if(node_PAL_current != NULL)
					goto DESTROY;

				link_PAL_current = link_previous;
				node_PAL_current = node_previous;
				continue; // no semicolon required
			}
		} else if(set_error(
			if_module_create_nodes(
				&i,
				&node_previous,
				parser))
		== 1) {	
			// OK
		} else if(set_error(
			if_call_create_nodes(
				&i,
				link_PAL_current,
				node_PAL_current,
				&node_previous,
				parser))
		== 1) {
			// OK
		}
		// check end of scope (period) or end of instruction (semicolon)
		if(set_error(
			if_scope_end_create_node(
				i,
				parser))
		== 1) {
			count_scope_nest -= 1;

			if(count_scope_nest == 0) {
				link_PAL_current = NULL;
				node_PAL_current = NULL;
			}

			node_previous = parser->nodes.top;
			i += 1;
		} else if(tokens[i].subtype == TokenSubtype_SEMICOLON) {
			i += 1;
		} else
			goto DESTROY;
		// error checking
		if(error == -1)
			goto DESTROY;
	}
	// destroy if there is no nodes
	if(parser->nodes.count == 1
	&& parser->nodes.first->memArea.count == 1)
		goto DESTROY;

	return true;
DESTROY:
	destroy_parser(parser);
	return false;
}

void destroy_parser(Parser* parser) {
	if(parser == NULL)
		return;

	parser_destroy_allocators(parser);
	initialize_parser(parser);
}

void parser_node_set_tail(
Node* node,
Node* tail) {
	assert(node != NULL);

	node->nodes[NODE_INDEX_TAIL] = tail;
}

Node* parser_node_get_tail(const Node* node) {
	assert(node != NULL);

	return node->nodes[NODE_INDEX_TAIL];
}
