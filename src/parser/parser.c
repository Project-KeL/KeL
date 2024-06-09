#include <assert.h>
#include <string.h>
#include "parser.h"
#include "parser_allocation.h"
#include "parser_error.h"
#include "parser_identifier.h"
#include "parser_utils.h"
#include <stdio.h>

static int error = 0;

static int set_error(int value) {
	if(error == -1)
		return -1;

	error = value;
	return value;
}

static bool if_scope_create_node(
size_t i,
Parser* parser) {
	if(!parser_is_scope_L((const Token*) parser->lexer->tokens.addr + i))
		return false;

	if(!parser_allocator_node(parser))
		return false;

	*((Node*) parser->nodes.top) = (Node) {
		.is_child = false,
		.type = NodeType_SCOPE_START,
		.subtype = NodeSubtypeScope_NO,
		.value = 0,
		.child = NULL};
	return true;
}

static int if_period_create_node(
size_t i,
Parser* parser) {
	const Token* token = (Token*) parser->lexer->tokens.addr + i;

	if(token->subtype != TokenSubtype_PERIOD)
		return 0;

	if(!parser_allocator_node(parser))
		return -1;

	Node* scope = parser_get_scope_from_period(parser);
	*((Node*) parser->nodes.top) = (Node) {
		.is_child = false,
		.type = NodeType_SCOPE_END,
		.subtype = scope->subtype};
	scope->child = (Node*) parser->nodes.top;
	return 1;
}

void initialize_parser(Parser* parser) {
	parser->lexer = NULL;
	parser_initialize_allocators(parser);
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
	bool is_scope = false;

	if(!parser_scan_errors(lexer))
		return false;

	if(!parser_create_allocators(parser))
		return false;

	while(i < lexer->tokens.count - 1) {
		// create nodes
		if(parser_is_scope_L(tokens + i)) {
			while(if_scope_create_node(
				i,
				parser)
			== true) {
				i += 1;
			}

			is_scope = true;
		} else if(set_error(
			if_identification_create_nodes(
				&i,
				memArea,
				parser))
		== 1) {
			if(parser_is_scope_L(tokens + i))
				is_scope = true;
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
		} else if(is_scope) {
			// OK
		} else
			goto DESTROY;
		// error checking
		if(error == -1)
			goto DESTROY;
		// allocation
		is_scope = false;
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
