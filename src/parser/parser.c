#include <assert.h>
#include <string.h>
#include "parser.h"
#include "parser_allocation.h"
#include "parser_error.h"
#include "parser_identifier.h"
#include "parser_utils.h"
#include <stdio.h>

// i is the current token to be processed

static int error = 0;

static int set_error(int value) {
	if(error == -1)
		return -1;

	error = value;
	return value;
}

static bool if_scope_create_node(
size_t i,
Parser* restrict parser) {
	if(parser_is_scope(
		i,
		parser->lexer)
	== false)
		return false;

	*((Node*) parser->nodes.top) = (Node) {
		.type = NodeType_SCOPE_START,
		.subtype = NodeSubtypeScope_NO,
		.value = 0};
	return true;
}

static int if_period_create_node(
bool allocate_period,
size_t i,
Parser* parser) {
	const Token* token = (Token*) parser->lexer->tokens.addr + i;

	if(token->subtype != TokenSubtype_PERIOD)
		return 0;

	if(allocate_period
	&& !parser_allocator(parser))
		return -1;

	Node* scope = parser_get_scope_from_period(parser);
	*((Node*) parser->nodes.top) = (Node) {
		.type = NodeType_SCOPE_END,
		.subtype = scope->subtype};
	scope->child = (Node*) parser->nodes.top;
	return 1;
}

void initialize_parser(Parser* parser) {
	parser->lexer = NULL;
	initialize_memory_chain(&parser->nodes);
}

bool create_parser(
const Lexer* lexer,
MemoryArea* memArea,
Parser* restrict parser) {
	assert(parser != NULL);
	assert(lexer != NULL);
	assert(memArea != NULL);

	parser->lexer = lexer;
	const Token* tokens = (const Token*) lexer->tokens.addr;
	size_t i = 1;
	bool allocate_period = false;

	if(!parser_scan_errors(lexer))
		return false;

	if(!parser_create_allocator(parser))
		return false;

	while(i < lexer->tokens.count - 1) {
		// create nodes
		if(parser_is_scope(
			i,
			parser->lexer)
		== true) {
			while(if_scope_create_node(
				i,
				parser)
			== true) {
				if(!parser_allocator(parser))
					goto DESTROY;

				i += 1;
			}
		} else if(set_error(
			if_identifier_create_nodes(
				&i,
				memArea,
				parser))
		== 1) {
			// OK
		} else if(((Node*) parser->nodes.previous)->type != NodeType_SCOPE_END)
			allocate_period = true;
		// check end of scope (period) or end of instruction (semicolon)
		if(set_error(
			if_period_create_node(
				allocate_period,
				i,
				parser))
		== 1) {
			allocate_period = false;
			i += 1;
		} else if(tokens[i].subtype == TokenSubtype_SEMICOLON) {
			i += 1;
		} else
			goto DESTROY;
		// error checking
		if(error == -1)
			goto DESTROY;
		// allocation
		if(!parser_allocator(parser))
			goto DESTROY;
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

void destroy_parser(Parser* restrict parser) {
	if(parser == NULL)
		return;

	parser->lexer = NULL;
	destroy_memory_chain(&parser->nodes);
}
