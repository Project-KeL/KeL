#include <assert.h>
#include <stddef.h>
#include "lexer_def.h"
#include "lexer_utils.h"
#include "parser_allocation.h"
#include "parser_identifier.h"
#include "parser_utils.h"
// WARNING: The following function is also defined as a temporary macro in "parser_def.h"
static NodeSubtypeKeyQualification token_subtype_QL_to_subtype(TokenSubtype subtype_token) {
	return (subtype_token & MASK_TOKEN_SUBTYPE_QL) >> (SHIFT_TOKEN_SUBTYPE_QL - 1);
}

static NodeSubtypeChildKeyType operator_modifier_to_subtype_left(TokenSubtype subtype_token) {
	switch(subtype_token) {
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

static NodeSubtypeChildKeyType operator_modifier_to_subtype_right(TokenSubtype subtype_token) {
	switch(subtype_token) {
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
				.subtype = operator_modifier_to_subtype_left(tokens[buffer_i].subtype),
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
			.subtype = operator_modifier_to_subtype_right(tokens[buffer_i].subtype),
			.token = &tokens[buffer_i],
			.child1 = NULL,
			.child2 = NULL};
		parent->child1 = &parser->nodes[buffer_j];
		parent = &parser->nodes[buffer_j];
		buffer_i += 1;
		buffer_j += 1;
	}

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
	NodeSubtypeKeyQualification subtype = NodeSubtypeKeyQualification_NO;
	// QL parsing
	if(tokens[buffer_i].type == TokenType_QL) {
		subtype = token_subtype_QL_to_subtype(tokens[buffer_i].subtype);
		buffer_i += 1;
	}
	// command parsing
	if(tokens[buffer_i].type != TokenType_COMMAND)
		return 0;

	if(tokens[buffer_i].subtype == TokenSubtype_HASH)
		subtype |= NodeSubtypeKeyQualification_HASH;
	else
		subtype |= NodeSubtypeKeyQualification_AT;

	buffer_i += 1;
	// create the beginning of the node
	if(tokens[buffer_i].type != TokenType_IDENTIFIER)
		return 0;

	if(parser_allocate_chunk(
		buffer_j + 1,
		parser)
	< 0)
		return -1;

	parser->nodes[buffer_j] = (Node) {
		.type = NodeType_IDENTIFICATION,
		.subtype = subtype, // command and qualifiers
		.token = &tokens[buffer_i]};
	buffer_i += 1;
	buffer_j += 1;
	// add the type as child nodes in `.child1`
	switch(if_type_create_nodes(
		&buffer_i,
		&buffer_j,
		&parser->nodes[buffer_j - 1],
		parser)
	) {
	case -1: return -1;
	case 0: return 0;
	case 1: /* fall through */;
	}

	*i = buffer_i;
	*j = buffer_j;
	return 1;
}

