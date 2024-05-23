#include <assert.h>
#include <stddef.h>
#include "lexer_def.h"
#include "parser_allocation.h"
#include "parser_identifier.h"
#include "parser_utils.h"
#include <stdio.h>
// WARNING: The following function is also defined as a temporary macro in "parser_def.h"
static NodeSubtypeKeyQualification token_subtype_QL_to_subtype(TokenSubtype subtype_token) {
	return (subtype_token & MASK_TOKEN_SUBTYPE_QL) >> (SHIFT_TOKEN_SUBTYPE_QL - 2);
}

static NodeSubtypeLiteral token_subtype_literal_to_subtype(TokenSubtype subtype_token) {
	return subtype_token;
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

static int type_allocate(
long int i,
long int j,
Parser* parser) {
	const char* code = parser->lexer->source->content;
	const Token* tokens = parser->lexer->tokens;
	long int count_node = 0; // parenthesis do not create nodes
	long int count_scope_parametered_nest = 0;

	do {
SCOPE_PARAMETERED:
		while(tokens[i].type == TokenType_L
		   && parser_is_operator_modifier(&tokens[i])) {
			i += 1;
			count_node += 1;
		}

		while(tokens[i].type == TokenType_R
		   && parser_is_operator_modifier(&tokens[i])) {
			i += 1;
			count_node += 1;
		}

		if(!parser_is_lock(&tokens[i]))
			return 0;

		i += 1;
		count_node += 1;

		if(tokens[i].subtype == TokenSubtype_LPARENTHESIS) {
LPARENTHESIS:
			i += 1;
			count_scope_parametered_nest += 1;
			goto READ_PARAMETER;
		} else if(tokens[i].subtype == TokenSubtype_COMMA) {
COMMA:
			i += 1;
READ_PARAMETER:
			if(!parser_is_lock(&tokens[i]))
				return 0;

			i += 1;

			if(tokens[i].subtype == TokenSubtype_LPARENTHESIS)
				goto LPARENTHESIS;
			else if(tokens[i].subtype == TokenSubtype_RPARENTHESIS)
				goto RPARENTHESIS;

			i += 1;
			count_node += 1;
			goto SCOPE_PARAMETERED;
		} else if(tokens[i].subtype == TokenSubtype_RPARENTHESIS) {
RPARENTHESIS:
			do {
				i += 1;
				count_scope_parametered_nest -= 1;
			} while(tokens[i].subtype == TokenSubtype_RPARENTHESIS);

			if(tokens[i].subtype == TokenSubtype_COMMA)
				goto COMMA;
		} else {
			while(tokens[i].type == TokenType_R
			   && parser_is_operator_leveling(&tokens[i])) {
				i += 1;
				count_node += 1;
			}	
		}
	} while(count_scope_parametered_nest != 0);

	if(parser_allocate_chunk(
		j + count_node,
		parser)
	== false)
		return -1;

	return 1;
}

static void type_bind_child_token(
NodeSubtype subtype,
const Token* token,
Node** parent,
Node* node) {
	*node = (Node) {
		.type = NodeType_CHILD,
		.subtype = subtype,
		.token = token,
		.child1 = NULL,
		.child2 = NULL};
	(*parent)->child1 = node;
	*parent = node;
}

static int if_type_create_nodes(
long int* i,
long int* j,
Node* parent,
Parser* parser) {
	const Token* tokens = parser->lexer->tokens;
	long int buffer_i = *i;
	long int buffer_j = *j;
	// allocation
	switch(type_allocate(
		buffer_i,
		buffer_j,
		parser)) {
	case -1: return -1;
	case 0: return 0;
	case 1: /*fall through*/;
	}

	long int count_scope_parametered_nest = 0;

	do {
SCOPE_PARAMETERED:
		// left side
		while(!parser_is_lock(&tokens[buffer_i])) {
				// do not support arrays yet
				// arrays may contain expression so this case will need to be processed
				// `.child2` will hold this expression
				type_bind_child_token(
					operator_modifier_to_subtype_left(tokens[buffer_i].subtype),
					&tokens[buffer_i],
					&parent,
					&parser->nodes[buffer_j]);
				// just ignore right brackets (for the moment)
				if(parent->subtype == NodeSubtypeChildKeyType_ARRAY)
					buffer_i += 1;

				buffer_i += 1;
				buffer_j += 1;
		}
		// lock
		type_bind_child_token(
			NodeSubtype_NO,
			&tokens[buffer_i],
			&parent,
			&parser->nodes[buffer_j]);
		buffer_i += 1;
		buffer_j += 1;
		// right side
		if(tokens[buffer_i].subtype == TokenSubtype_LPARENTHESIS) {
LPARENTHESIS:
			buffer_i += 1;
			count_scope_parametered_nest += 1;
			goto READ_PARAMETER;
		} else if(tokens[buffer_i].subtype == TokenSubtype_COMMA) {
COMMA:
			buffer_i += 1;
READ_PARAMETER:
			if(!parser_is_lock(&tokens[buffer_i]))
				goto SCOPE_PARAMETERED;

			type_bind_child_token(
				NodeSubtype_NO,
				&tokens[buffer_i],
				&parent,
				&parser->nodes[buffer_j]);
			buffer_i += 1;
			buffer_j += 1;

			if(tokens[buffer_i].subtype == TokenSubtype_LPARENTHESIS)
				goto LPARENTHESIS;
			if(tokens[buffer_i].subtype == TokenSubtype_RPARENTHESIS)
				goto RPARENTHESIS;

			buffer_i += 1;
			goto SCOPE_PARAMETERED;
		} else if(tokens[buffer_i].subtype == TokenSubtype_RPARENTHESIS) {
RPARENTHESIS:
			do {
				buffer_i += 1;
				count_scope_parametered_nest -= 1;
			} while(tokens[buffer_i].subtype == TokenSubtype_RPARENTHESIS);

			if(tokens[buffer_i].subtype == TokenSubtype_COMMA)
				goto COMMA;
		} else {
			while(tokens[buffer_i].type == TokenType_R) {
				type_bind_child_token(
					operator_modifier_to_subtype_right(tokens[buffer_i].subtype),
					&tokens[buffer_i],
					&parent,
					&parser->nodes[buffer_j]);
				buffer_i += 1;
				buffer_j += 1;
			}
		}
	} while(count_scope_parametered_nest != 0);

	*i = buffer_i;
	*j = buffer_j;
	return 1;
}

static int if_initialization_create_node(
long int* i,
long int* j,
Node* parent,
Parser* parser) {
	// just parse literals for the moment, expressions later
	const Token* tokens = parser->lexer->tokens;
	long int buffer_i = *i;
	long int buffer_j = *j;

	if(tokens[buffer_i].type != TokenType_LITERAL)
		return 0;

	if(parser_allocate_chunk(
		buffer_j + 1,
		parser)
	== false)
		return -1;

	parser->nodes[buffer_j] = (Node) {
		.type = NodeType_LITERAL,
		.subtype = token_subtype_literal_to_subtype(tokens[buffer_i].subtype),
		.token = &tokens[buffer_i]};
	parent->child1 = &parser->nodes[buffer_j];
	*i = buffer_i + 1;
	*j = buffer_j + 1;
	return 1;
}

int if_identifier_create_nodes(
long int* i,
long int* j,
Parser* parser) {
	const Token* tokens = parser->lexer->tokens;
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
		subtype |= NodeSubtypeKeyIdentificationBitCommand_HASH;
	else
		subtype |= NodeSubtypeKeyIdentificationBitCommand_AT;

	buffer_i += 1;
	// create the beginning of the node
	if(tokens[buffer_i].type != TokenType_IDENTIFIER)
		return 0;

	if(parser_allocate_chunk(
		buffer_j + 1,
		parser)
	== false)
		return -1;

	parser->nodes[buffer_j] = (Node) { // `buffer_j` still equals `*j`
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

	switch(if_initialization_create_node(
		&buffer_i,
		&buffer_j,
		&parser->nodes[buffer_j - 1],
		parser)) {
	case -1: return -1;
	case 0: // declaration case
		parser->nodes[*j].subtype |= NodeSubtypeKeyIdentificationBitType_DECLARATION;
		break;
	case 1: // initialization case
		parser->nodes[*j].subtype |= NodeSubtypeKeyIdentificationBitType_INITIALIZATION;
		break;
	}

	*i = buffer_i;
	*j = buffer_j;
	return 1;
}

