#include <assert.h>
#include <string.h>
#include "parser.h"
#include "parser_allocation.h"
#include "parser_error.h"
#include "parser_identifier.h"
#include "parser_utils.h"
#include <stdio.h>

// i is the current token to be processed
// j is the current node to be created

static int error = 0;

static int set_error(int value) {
	if(error == -1)
		return -1;

	error = value;
	return value;
}

static void create_node_null(Node* node) {
	*node = (Node) {
		.type = NodeType_NO,
		.subtype = NodeSubtype_NO,
		.token = NULL,
		.child = NULL};
}

static bool if_scope_create_node(
size_t i,
size_t j,
Parser* restrict parser) {
	if(parser_is_scope(
		i,
		parser->lexer)
	== false)
		return false;

	parser->nodes[j] = (Node) {
		.type = NodeType_SCOPE_START,
		.subtype = NodeSubtypeScope_NO};
	return true;
}

static int if_period_create_node(
size_t i,
size_t j,
Parser* parser) {
	const Token* tokens = (Token*) parser->lexer->tokens.addr;

	if(tokens[i].subtype != TokenSubtype_PERIOD)
		return false;

	size_t j_scope_start = parser_get_j_scope_start_from_end(
			j,
			parser);
	parser->nodes[j] = (Node) {
		.type = NodeType_SCOPE_END,
		.subtype = parser->nodes[j_scope_start].subtype};
	parser->nodes[j_scope_start].child = &parser->nodes[j];
	return true;
}

void initialize_parser(Parser* parser) {
	parser->lexer = NULL;
	parser->nodes = NULL;
	parser->count = 0;
}

bool create_parser(
const Lexer* lexer,
MemoryArea* memArea,
Parser* restrict parser) {
	if(lexer->tokens.count == 0)
		goto DESTROY;

	parser->lexer = lexer;
	parser->nodes = NULL;
	parser->count = 0;
	const Token* tokens = (Token*) lexer->tokens.addr;
	size_t i = 1;
	size_t j = 1;

	if(!parser_scan_errors(lexer))
		return false;

	if(parser_allocate_chunk(
		1,
		parser)
	== false)
		return false;

	create_node_null(&parser->nodes[0]);

	while(i < lexer->tokens.count - 1) {
		// allocation
		if(parser_allocate_chunk(
			j + 1,
			parser)
		== false)
			goto DESTROY;
		// create nodes
		if(if_scope_create_node(
			i,
			j,
			parser)
		== true) {
			i += 1;
			j += 1;
		} else if(set_error(
			if_identifier_create_nodes(
				&i,
				&j,
				memArea,
				parser))
		== 1) {
			// OK
		}
		// check end of scope (period) or end of instruction (semicolon)
		if(set_error(
			if_period_create_node(
				i,
				j,
				parser))
		== 1) {
			i += 1;
			j += 1;
		} else if(tokens[i].subtype == TokenSubtype_SEMICOLON) {
			i += 1;
		} else
			goto DESTROY;
		// error checking
		if(error == -1)
			goto DESTROY;
		// unlike `create_lexer` all the incrementations are done
	}

	if(j == 1)
		goto DESTROY;

	parser->count = j;
	Node* nodes_realloc = realloc(
		parser->nodes,
		(j + 1) * sizeof(Node));

	if(nodes_realloc == NULL) {
DESTROY:
		destroy_parser(parser);
		return false;
	}

	parser->nodes = nodes_realloc;
	create_node_null(&parser->nodes[j]);
	return true;
}

void destroy_parser(Parser* restrict parser) {
	if(parser == NULL)
		return;

	parser->lexer = NULL;
	free(parser->nodes);
	parser->nodes = NULL;
	parser->count = 0;
}
