#include <assert.h>
#include <stddef.h>
#include "lexer_def.h"
#include "parser_allocation.h"
#include "parser_identifier.h"
#include "parser_utils.h"
#include <stdio.h>
// WARNING: The following function is also defined as a temporary macro in "parser_def.h"
static NodeSubtypeKeyQualification token_subtype_QL_to_subtype(TokenSubtype subtype_token) {
	return (subtype_token & MASK_TOKEN_SUBTYPE_QL) >> (SHIFT_TOKEN_SUBTYPE_QL - 3);
}

static NodeSubtypeLiteral token_subtype_literal_to_subtype(TokenSubtype subtype_token) {
	return subtype_token;
}

static NodeSubtypeChildKeyTypeModifier operator_modifier_to_subtype_left(TokenSubtype subtype_token) {
	switch(subtype_token) {
	case TokenSubtype_AMPERSAND: return NodeSubtypeChildKeyTypeModifier_AMPERSAND_LEFT;
	// brackets are always at the left side of the lock
	case TokenSubtype_LBRACKET: return NodeSubtypeChildKeyTypeModifier_ARRAY;
	case TokenSubtype_RBRACKET: return NodeSubtypeChildKeyTypeModifier_ARRAY;
	case TokenSubtype_MINUS: return NodeSubtypeChildKeyTypeModifier_MINUS_LEFT;
	case TokenSubtype_PIPE: return NodeSubtypeChildKeyTypeModifier_PIPE_LEFT;
	case TokenSubtype_PLUS: return NodeSubtypeChildKeyTypeModifier_PLUS_LEFT;
	default: assert(false);
	}
}

static NodeSubtypeChildKeyTypeModifier operator_modifier_to_subtype_right(TokenSubtype subtype_token) {
	switch(subtype_token) {
	case TokenSubtype_AMPERSAND: return NodeSubtypeChildKeyTypeModifier_AMPERSAND_RIGHT;
	case TokenSubtype_MINUS: return NodeSubtypeChildKeyTypeModifier_MINUS_RIGHT;
	case TokenSubtype_PIPE: return NodeSubtypeChildKeyTypeModifier_PIPE_RIGHT;
	case TokenSubtype_PLUS: return NodeSubtypeChildKeyTypeModifier_PLUS_RIGHT;
	default: assert(false);
	}
}

static void type_bind_child_token(
NodeTypeChildKeyType type,
NodeSubtype subtype,
const Token* token,
Node** parent,
Node* node) {
	*node = (Node) {
		.type = type,
		.subtype = subtype,
		.token = token,
		.child1 = NULL,
		.child2 = NULL};
	(*parent)->child1 = node;
	*parent = node;
}

static bool parse_type_lock_left(
long int* i,
long int* j,
Node* parent,
Parser* parser) {
	long int buffer_i = *i;
	long int buffer_j = *j;
	const Token* tokens = parser->lexer->tokens;
	Node* nodes = parser->nodes;
	// L side before lock
	while(tokens[buffer_i].type == TokenType_L
	   && parser_is_operator_modifier(&tokens[buffer_i])) {
			// do not support arrays yet
			// arrays may contain expression so this case will need to be processed
			// `.child2` will hold this expression
			type_bind_child_token(
				NodeTypeChildKeyType_MODIFIER,
				operator_modifier_to_subtype_left(tokens[buffer_i].subtype),
				&tokens[buffer_i],
				&parent,
				&nodes[buffer_j]);
			// just ignore right brackets (for the moment)
			if(parent->subtype == NodeSubtypeChildKeyTypeModifier_ARRAY)
				buffer_i += 1;

			buffer_i += 1;
			buffer_j += 1;
	}
	// R side before lock
	while(tokens[buffer_i].type == TokenType_R
	   && parser_is_operator_modifier(&tokens[buffer_i])) {
		type_bind_child_token(
			NodeTypeChildKeyType_MODIFIER,
			operator_modifier_to_subtype_right(tokens[buffer_i].subtype),
			&tokens[buffer_i],
			&parent,
			&nodes[buffer_j]);

		if(parent->subtype == NodeSubtypeChildKeyTypeModifier_ARRAY)
			buffer_i += 1;

		buffer_i += 1;
		buffer_j += 1;
	}

	if(tokens[buffer_i].type != TokenType_R)
		return false;

	*i = buffer_i;
	*j = buffer_j;
	return true;
}

static bool parse_type_lock_right(
long int* i,
long int* j,
Node* parent,
Parser* parser) {
	long int buffer_i = *i;
	long int buffer_j = *j;
	const Token* tokens = parser->lexer->tokens;
	Node* nodes = parser->nodes;

	while(tokens[buffer_i].type == TokenType_R
	   && parser_is_operator_leveling(&tokens[buffer_i])) {
		type_bind_child_token(
			NodeTypeChildKeyType_MODIFIER,
			operator_modifier_to_subtype_right(tokens[buffer_i].subtype),
			&tokens[buffer_i],
			&parent,
			&nodes[buffer_j]);
		buffer_i += 1;
		buffer_j += 1;
	}

	if(parser_is_operator_modifier(&tokens[buffer_i]))
		return false;

	*i = buffer_i;
	*j = buffer_j;
	return true;
}

static int if_type_create_nodes(
long int* i,
long int* j,
Node* parent,
Allocator* allocator,
Parser* parser) {
	const Token* tokens = parser->lexer->tokens;
	long int buffer_i = *i;
	long int buffer_j = *j;
	// rough allocation
	while(tokens[buffer_i].subtype != TokenSubtype_SEMICOLON
	   && buffer_i < parser->lexer->count) {
		buffer_i += 1;
		buffer_j += 1;
	}

	if(parser_allocate_chunk(
		buffer_j,
		parser)
	== false)
		return -1;

	buffer_i = *i;
	buffer_j = *j;
	// are there parameters in this scoped type: allocator->address[count_parenthesis_nest]
	long int count_parenthesis_nest = 0;
	// nodes = parser->nodes[buffer_j] may be a bad idea because of the labels
	if(parser_is_R_left_parenthesis(&tokens[buffer_i]))
		goto R_LPARENTHESIS_SKIP_PARAMETER;

	do {
		// lock alone
TYPE:
		long int j_lock;

		if(parse_type_lock_left(
			&buffer_i,
			&buffer_j,
			parent,
			parser)
		== false)
			return false;

		if(tokens[buffer_i].subtype != TokenSubtype_LPARENTHESIS) {
			type_bind_child_token(
				NodeTypeChildKeyType_LOCK,
				NodeSubtypeChild_NO,
				&tokens[buffer_i],
				&parent,
				&parser->nodes[buffer_j]);
			j_lock = buffer_j;
			buffer_i += 1;
			buffer_j += 1;
			allocator->address[count_parenthesis_nest] = 1;

			if(parse_type_lock_right(
				&buffer_i,
				&buffer_j,
				parent,
				parser)
			== false)
				return 0;

			if(tokens[buffer_i].subtype != TokenSubtype_LPARENTHESIS
			&& count_parenthesis_nest == 0)
				break;
		}
		// lock not alone (good luck)
		while(parser_is_R_left_parenthesis(&tokens[buffer_i])) {
R_LPARENTHESIS:
			if(count_parenthesis_nest > 1
			// case `:(a :())`
			|| (count_parenthesis_nest == 1
			 && !parser_is_parenthesis(&tokens[buffer_i - 1])))
				goto R_LPARENTHESIS_SKIP_PARAMETER;

			if(!parser_is_key(&tokens[buffer_i]))
				return false;

			type_bind_child_token(
				NodeTypeChildKeyType_LOCK,
				NodeSubtypeChildKeyTypeScoped_PARAMETER,
				&tokens[buffer_i],
				&parent,
				&parser->nodes[buffer_j]);
			buffer_i += 1;
			buffer_j += 1;
R_LPARENTHESIS_SKIP_PARAMETER:
			type_bind_child_token(
				NodeTypeChildKeyType_LOCK,
				NodeSubtypeChildKeyTypeScoped_RETURN_NONE,
				NULL,
				&parent,
				&parser->nodes[buffer_j]);
			j_lock = buffer_j;
			buffer_i += 1;
			buffer_j += 1;
			allocator->address[count_parenthesis_nest] = 1;
			count_parenthesis_nest += 1;
			allocator->address[count_parenthesis_nest] = 0;
		}

		if(tokens[buffer_i].subtype == TokenSubtype_LPARENTHESIS) {
			parser->nodes[j_lock].subtype = NodeSubtypeChildKeyTypeScoped_RETURN_LOCK;
LPARENTHESIS:
			// case like `:(())`
			if(tokens[buffer_i - 1].subtype == TokenSubtype_LPARENTHESIS)
				return 0;

			buffer_i += 1;
			count_parenthesis_nest += 1;
			allocator->address[count_parenthesis_nest] = 0;

			if(parser_is_R_left_parenthesis(&tokens[buffer_i]))
				goto R_LPARENTHESIS;

			goto READ_PARAMETER;
		} else if(tokens[buffer_i].subtype == TokenSubtype_COMMA) {
COMMA:
			buffer_i += 1;
READ_PARAMETER:
			if(tokens[buffer_i - 1].subtype == TokenSubtype_LPARENTHESIS
			&& tokens[buffer_i].subtype == TokenSubtype_RPARENTHESIS)
				goto RPARENTHESIS;

			allocator->address[count_parenthesis_nest] = 1;

			if(parser_is_key(&tokens[buffer_i])) {
				// ignore parameter if nested
				if(count_parenthesis_nest > 1) {
					buffer_i += 1;
					goto TYPE;
				}

				type_bind_child_token(
					NodeTypeChildKeyType_LOCK,
					NodeSubtypeChildKeyTypeScoped_PARAMETER,
					&tokens[buffer_i],
					&parent,
					&parser->nodes[buffer_j]);
				buffer_i += 1;
				buffer_j += 1;
				goto TYPE;
			} else if(count_parenthesis_nest > 1
			       && parser_is_lock(&tokens[buffer_i])) {
				goto TYPE;
			} else {
				// a lock must succeed a key if there is more than one nesting level
				return 0;
			}
		} else if(tokens[buffer_i].subtype == TokenSubtype_RPARENTHESIS) {
RPARENTHESIS:
			if(allocator->address[count_parenthesis_nest] == 0) {
				type_bind_child_token(
					NodeTypeChildKeyType_LOCK,
					NodeSubtypeChildKeyTypeScoped_PARAMETER_NONE,
					NULL,
					&parent,
					&parser->nodes[buffer_j]);
				buffer_j += 1;
			}

			do {
				buffer_i += 1;
				count_parenthesis_nest -= 1;
			} while(tokens[buffer_i].subtype == TokenSubtype_RPARENTHESIS);

			if(tokens[buffer_i].subtype == TokenSubtype_COMMA)
				goto COMMA;
		} else if(parser_is_key(&tokens[buffer_i])) {
				// case `:(a :b)`
				if(count_parenthesis_nest > 1) {
					buffer_i += 1;
					goto TYPE;
				}

				type_bind_child_token(
					NodeTypeChildKeyType_LOCK,
					NodeSubtypeChildKeyTypeScoped_PARAMETER,
					&tokens[buffer_i],
					&parent,
					&parser->nodes[buffer_j]);
				buffer_i += 1;
				buffer_j += 1;
				goto TYPE;
		} else if(parser_is_lock(&tokens[buffer_i])) {
			goto TYPE;
		} else {
			return 0;
		}
	} while(count_parenthesis_nest != 0);

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
Allocator* allocator,
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
		.subtype = subtype, // command, qualifiers and scoped
		.token = &tokens[buffer_i]};
	buffer_i += 1;
	buffer_j += 1;
	// add the type as child nodes in `.child1`
	switch(if_type_create_nodes(
		&buffer_i,
		&buffer_j,
		&parser->nodes[buffer_j - 1],
		allocator,
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

