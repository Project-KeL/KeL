#include <assert.h>
#include <string.h>
#include "parser.h"
#include "parser_allocator.h"
#include "parser_call.h"
#include "parser_error.h"
#include "parser_identifier.h"
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
	// the counter is triggered when the first parameterized label is encountered
	size_t count_scope_nest = 0;
	// to insert declarations in the right place
	const MemoryChain buffer_memChain = parser->nodes;
	parser->nodes = parser->declarations;

	while(i < parser->lexer->tokens.count - 1) {
		Node* node_identification;

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
			if_declaration_create_nodes(
				&i,
				memArea,
				&node_identification,
				parser))
		== 1) {
			if((node_identification->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_SCOPED)
			== NodeSubtypeIdentificationBitScoped_LABEL_PARAMETERIZED)
				count_scope_nest += 1;
		} else
			i += 1;

		if(error == -1)
			return false;
	}

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
	Node* buffer_node = NULL;
	Node* buffer_node_previous = NULL;

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
		if(set_error(
			if_module_create_nodes(
				&i,
				parser))
		== 1) {
			// OK
		} else if(parser_is_scope_L(tokens + i)) {
			while(if_scope_create_node(
				i,
				parser)
			== true) {
				buffer_node = parser->nodes.top;

				if(buffer_node_previous != NULL
				&& (buffer_node_previous->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_SCOPED))
					buffer_node_previous->child2 = buffer_node;

				i += 1;
			}
		} else if(set_error(
			if_identification_create_nodes(
				&i,
				memArea,
				&buffer_node,
				parser))
		== 1) {
			// OK
		} else if(set_error(
			if_call_create_nodes(
				&i,
				parser))
		== 1) {
			// OK
		}
		// check end of scope (period) or end of instruction (semicolon)
		if(set_error(
			if_period_create_node(
				i,
				parser))
		== 1) {
			i += 1;
		} else if(tokens[i].subtype == TokenSubtype_SEMICOLON) {
			i += 1;
		} else if(buffer_node->type == NodeType_SCOPE_START
		       || (buffer_node->type == NodeType_IDENTIFICATION
		        && parser_is_scope_L(tokens + i))) {
			// OK
		} else
			goto DESTROY;
		// error checking
		if(error == -1)
			goto DESTROY;

		buffer_node_previous = buffer_node;
	}
/*
	if(j == 1)
		goto DESTROY;
*/
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
