#include <assert.h>
#include <stddef.h>
#include "lexer_def.h"
#include "lexer_utils.h"
#include "parser_allocation.h"
#include "parser_identifier.h"
#include "parser_utils.h"

static NodeSubtypeChildKeyType operator_modifiers_to_subtype_left(TokenSubtype token_subtype) {
	switch(token_subtype) {
	case TokenSubtype_AMPERSAND: return NodeSubtypeChildKeyType_AMPERSAND_LEFT;
	// brackets are always at the left side of the lock
	case TokenSubtype_LBRACKET: return NodeSubtypeChildKeyType_ARRAY;
	case TokenSubtype_RBRACKET: return NodeSubtypeChildKeyType_ARRAY;
	case TokenSubtype_MINUS: return NodeSubtypeChildKeyType_MINUS_LEFT;
	case TokenSubtype_PIPE: return NodeSubtypeChildKeyType_PIPE_LEFT;
	case TokenSubtype_PLUS: return NodeSubtypeChildKeyType_PLUS_LEFT;
	default: assert(false);
	}
}

static NodeSubtypeChildKeyType operator_modifiers_to_subtype_right(TokenSubtype token_subtype) {
	switch(token_subtype) {
	case TokenSubtype_AMPERSAND: return NodeSubtypeChildKeyType_AMPERSAND_RIGHT;
	case TokenSubtype_MINUS: return NodeSubtypeChildKeyType_MINUS_RIGHT;
	case TokenSubtype_PIPE: return NodeSubtypeChildKeyType_PIPE_RIGHT;
	case TokenSubtype_PLUS: return NodeSubtypeChildKeyType_PLUS_RIGHT;
	default: assert(false);
	}
}

static int if_type_create_nodes(
long int* i,
long int* j,
Node* parent,
Parser* parser) {
	const Token* tokens = parser->lexer->tokens;
	long int buffer_i = *i;
	long int buffer_j = *j;
	// allocate modifier parts of the type
	{
		long int count_tokens = 0;

		while(tokens[buffer_i + count_tokens].type == TokenType_L
		   && parser_is_operator_modifier(&tokens[buffer_i + count_tokens])) count_tokens += 1;

		while(tokens[buffer_i + count_tokens].type == TokenType_R
		   && parser_is_operator_modifier(&tokens[buffer_i + count_tokens])) count_tokens += 1;

		if(!parser_is_lock(&tokens[buffer_i + count_tokens]))
			return 0;

		count_tokens += 1;

		while(tokens[buffer_i + count_tokens].type == TokenType_R
		   && parser_is_operator_leveling(&tokens[buffer_i + count_tokens])) count_tokens += 1;

		if(parser_allocate_chunk(
			buffer_j + count_tokens,
			parser)
		== false)
			return -1;
	}
	// left side
	while(!parser_is_lock(&tokens[buffer_i])) {
			// do not support arrays yet
			// arrays may contain expression so this case will need to be processed
			// `.child2` will hold this expression
			parser->nodes[buffer_j] = (Node) {
				.type = NodeType_CHILD,
				.subtype = operator_modifiers_to_subtype_left(tokens[buffer_i].subtype),
				.token = &tokens[buffer_i],
				.child1 = NULL,
				.child2 = NULL};
			// the type must be binded with another node (declaration, initialization, previous node, ...)
			parent->child1 = &parser->nodes[buffer_j];
			parent = &parser->nodes[buffer_j];
			// just ignore right brackets (for the moment)
			if(parent->subtype == NodeSubtypeChildKeyType_ARRAY)
				buffer_i += 1;

			buffer_i += 1;
			buffer_j += 1;
	}
	// lock
	parser->nodes[buffer_j] = (Node) {
		.type = NodeType_CHILD,
		.subtype = NodeSubtype_NO,
		.token = &tokens[buffer_i],
		.child1 = NULL,
		.child2 = NULL};
	parent->child1 = &parser->nodes[buffer_j];
	parent = &parser->nodes[buffer_j];
	buffer_i += 1;
	buffer_j += 1;
	// right side
	while(tokens[buffer_i].type == TokenType_R) {
		parser->nodes[buffer_j] = (Node) {
			.type = NodeType_CHILD,
			.subtype = operator_modifiers_to_subtype_right(tokens[buffer_i].subtype),
			.token = &tokens[buffer_i],
			.child1 = NULL,
			.child2 = NULL};
		parent->child1 = &parser->nodes[buffer_j];
		parent = &parser->nodes[buffer_j];
		buffer_i += 1;
		buffer_j += 1;
	}
	// we read the next token but 
	*i = buffer_i;
	*j = buffer_j;
	return 1;
}

int if_identifier_create_nodes(
long int* i,
long int* j,
Parser* parser) {
	Token* tokens = parser->lexer->tokens;
	long int buffer_i = *i;
	long int buffer_j = *j;

	if(tokens[buffer_i].subtype != TokenSubtype_AT)
		return 0;

	buffer_i += 1;

	if(tokens[buffer_i].type != TokenType_IDENTIFIER)
		return 0;

	if(parser_allocate_chunk(
		buffer_j + 1,
		parser)
	< 0)
		return -1;

	parser->nodes[buffer_j] = (Node) {
		.type = NodeType_DECLARATION,
		.subtype = NodeSubtype_NO,
		.token = &tokens[buffer_i]};
	buffer_i += 1;
	buffer_j += 1;

	switch(if_type_create_nodes(
		&buffer_i,
		&buffer_j,
		&parser->nodes[buffer_j - 1],
		parser)
	) {
	case -1: return -1;
	case 0: return 0;
	}

	*i = buffer_i;
	*j = buffer_j;
	return 1;
}

