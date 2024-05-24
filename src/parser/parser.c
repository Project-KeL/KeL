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

static long int error = 0;

static int set_error(long int value) {
	if(value == -1)
		return -1;

	error = value;
	return value;
}

static bool if_scope_create_node(
long int i,
long int j,
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
long int i,
long int j,
Parser* parser) {
	if(parser->lexer->tokens[i].subtype != TokenSubtype_PERIOD)
		return false;

	long int j_scope_start = parser_get_j_scope_start_from_end(
			j,
			parser);
	parser->nodes[j] = (Node) {
		.type = NodeType_SCOPE_END,
		.subtype = parser->nodes[j_scope_start].subtype};
	parser->nodes[j_scope_start].child = &parser->nodes[j];
	return true;
}

bool create_parser(
const Lexer* lexer,
Allocator* allocator,
Parser* restrict parser) {
	assert(lexer != NULL);
	assert(lexer->tokens != NULL);
	parser->lexer = lexer;
	parser->nodes = NULL;
	parser->count = 0;
	const char* code = lexer->source->content;
	const Token* tokens = lexer->tokens;
	long int i = 0;
	long int j = 0;

	if(!parser_scan_errors(lexer))
		return false;

	while(i < lexer->count) {
		// allocation
		if(parser_allocate_chunk(
			i,
			parser)
		== false) {
			destroy_parser(parser);
			return false;
		}
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
				allocator,
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
			continue; // no new node
		} else {
			destroy_parser(parser);
			return false;
		}
		// error checking
		if(error == -1) {
			destroy_parser(parser);
			return false;
		}
		// unlike `create_lexer` all the incrementations are done
	}

	if(j == 0) {
		destroy_parser(parser);
		return false;
	}

	parser->count = j;
	Node* nodes_realloc = realloc(
		parser->nodes,
		(parser->count + 1) * sizeof(Node));

	if(nodes_realloc == NULL) {
		destroy_parser(parser);
		return false;
	}

	parser->nodes = nodes_realloc;
	parser->nodes[parser->count] = (Node) {.type = NodeType_NO};
	return true;
}

void destroy_parser(Parser* restrict parser) {
	parser->lexer = NULL;
	free(parser->nodes);
	parser->nodes = NULL;
	parser->count = 0;
}
