#include <assert.h>
#include <string.h>
#include "parser.h"
#include "parser_error.h"
#include "parser_utils.h"
#include "parser_utils_core.h"
#include <stdio.h>

// i is the current token to be processed
// j is the current node to be created

static bool if_scope_create_node(
long int i,
long int j,
Parser* restrict parser) {
	if(is_scope(
		i,
		parser->lexer)
	== false)
		return false;

	parser->nodes[j] = (Node) {
		.type = NodeType_SCOPE_START,
		.value = NodeTypeScope_NO};
	return true;
}

static bool if_period_create_node(
long int i,
long int j,
Parser* restrict parser) {
	if(parser->lexer->tokens[i].subtype != TokenSubtype_PERIOD)
		return false;

	parser->nodes[j] = (Node) {.type = NodeType_SCOPE_END};
	return true;
}

bool create_parser(
const Lexer* lexer,
Allocator* restrict allocator,
Parser* restrict parser) {
	assert(lexer != NULL);
	assert(lexer->tokens != NULL);
	assert(lexer->tokens[lexer->count - 1].type == TokenType_NO);
	parser->lexer = lexer;
	parser->nodes = NULL;
	parser->count = 0;
	const char* code = lexer->source->content;
	const Token* tokens = lexer->tokens;
	long int count_scope = 0;
	long int i = 0;
	long int j = 0;

	if(!parser_scan_errors(lexer))
		return false;
#define NODES_CHUNK 4096
	while(i < lexer->count - 1) { // null token
		// allocation
		if(parser->count <= i) {
			Node* nodes_realloc = realloc(
				parser->nodes,
				(parser->count + NODES_CHUNK) * sizeof(Node));

			if(nodes_realloc == NULL) {
				destroy_parser(parser);
				return false;
			}

			parser->nodes = nodes_realloc;
			parser->count += NODES_CHUNK;
		}
		// create nodes
		if(if_scope_create_node(
			i,
			j,
			parser)
		== true) {
			count_scope += 1;
		// check end of scope (period) or end of instruction (semicolon)
		} else if(if_period_create_node(
			i,
			j,
			parser)
		== true) {
			count_scope -= 1;
		} else if(tokens[i].subtype == TokenSubtype_SEMICOLON) {
			i += 1;
			continue; // no new node
		} else {
			destroy_parser(parser);
			return false;
		}
		// loop end
		i += 1;
		j += 1;
	}

	parser->count = j + 1; // null node
	Node* nodes_realloc = realloc(
		parser->nodes,
		parser->count);
#undef NODES_CHUNK // no more allocation reminder
	if(nodes_realloc == NULL) {
		destroy_parser(parser);
		return false;
	}

	parser->nodes = nodes_realloc;
	parser->nodes[parser->count - 1] = (Node) {
		.type = NodeType_NO,
		.child = NULL}; // null node
	return true;
}

void destroy_parser(Parser* restrict parser) {
	parser->lexer = NULL;
	free(parser->nodes);
	parser->nodes = NULL;
	parser->count = 0;
}
