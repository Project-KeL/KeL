#include <assert.h>
#include <stddef.h>
#include "lexer_def.h"
#include "parser_allocation.h"
#include "parser_identifier.h"
#include "parser_utils.h"
#include <stdio.h>

static NodeSubtypeIdentificationBitCommand token_subtype_command_to_subtype(TokenSubtype subtype_token) {
	return subtype_token == TokenSubtype_HASH ?
		NodeSubtypeIdentificationBitCommand_HASH
		: NodeSubtypeIdentificationBitCommand_AT;
}

static NodeSubtypeLiteral token_subtype_literal_to_subtype(TokenSubtype subtype_token) {
	return (NodeSubtypeLiteral) subtype_token;
}

static NodeSubtypeChildTypeModifier operator_modifier_to_subtype_left(TokenSubtype subtype_token) {
	switch(subtype_token) {
	case TokenSubtype_AMPERSAND: return NodeSubtypeChildTypeModifier_AMPERSAND_LEFT;
	// brackets are always at the left side of the lock
	case TokenSubtype_LBRACKET: return NodeSubtypeChildTypeModifier_ARRAY;
	case TokenSubtype_RBRACKET: return NodeSubtypeChildTypeModifier_ARRAY;
	case TokenSubtype_MINUS: return NodeSubtypeChildTypeModifier_MINUS_LEFT;
	case TokenSubtype_PIPE: return NodeSubtypeChildTypeModifier_PIPE_LEFT;
	case TokenSubtype_PLUS: return NodeSubtypeChildTypeModifier_PLUS_LEFT;
	default: assert(false);
	}
}

static NodeSubtypeChildTypeModifier operator_modifier_to_subtype_right(TokenSubtype subtype_token) {
	switch(subtype_token) {
	case TokenSubtype_AMPERSAND: return NodeSubtypeChildTypeModifier_AMPERSAND_RIGHT;
	case TokenSubtype_MINUS: return NodeSubtypeChildTypeModifier_MINUS_RIGHT;
	case TokenSubtype_PIPE: return NodeSubtypeChildTypeModifier_PIPE_RIGHT;
	case TokenSubtype_PLUS: return NodeSubtypeChildTypeModifier_PLUS_RIGHT;
	default: assert(false);
	}
}

static void type_bind_child_token(
NodeTypeChildType type,
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
size_t* i,
size_t* j,
Node* parent,
Parser* parser) {
	size_t buffer_i = *i;
	size_t buffer_j = *j;
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;
	Node* nodes = parser->nodes;
	// L side before lock
	while(tokens[buffer_i].type == TokenType_L
	   && parser_is_operator_modifier(&tokens[buffer_i])) {
			// do not support arrays yet
			// arrays may contain expression so this case will need to be processed
			// `.child2` will hold this expression
			type_bind_child_token(
				NodeTypeChildType_MODIFIER,
				(NodeSubtype) operator_modifier_to_subtype_left(tokens[buffer_i].subtype),
				&tokens[buffer_i],
				&parent,
				&nodes[buffer_j]);
			// just ignore right brackets (for the moment)
			if(parent->subtype == NodeSubtypeChildTypeModifier_ARRAY)
				buffer_i += 1;

			buffer_i += 1;
			buffer_j += 1;
	}
	// R side before lock
	while(tokens[buffer_i].type == TokenType_R
	   && parser_is_operator_modifier(&tokens[buffer_i])) {
		type_bind_child_token(
			NodeTypeChildType_MODIFIER,
			(NodeSubtype) operator_modifier_to_subtype_right(tokens[buffer_i].subtype),
			&tokens[buffer_i],
			&parent,
			&nodes[buffer_j]);

		if(parent->subtype == NodeSubtypeChildTypeModifier_ARRAY)
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
size_t* i,
size_t* j,
Node* parent,
Parser* parser) {
	size_t buffer_i = *i;
	size_t buffer_j = *j;
	const Token* tokens = (const Token*) &parser->lexer->tokens.addr;
	Node* nodes = parser->nodes;

	while(tokens[buffer_i].type == TokenType_R
	   && parser_is_operator_leveling(&tokens[buffer_i])) {
		type_bind_child_token(
			NodeTypeChildType_MODIFIER,
			(NodeSubtype) operator_modifier_to_subtype_right(tokens[buffer_i].subtype),
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
size_t* i,
size_t* j,
Node* parent,
MemoryArea* memArea,
Parser* parser) {
	char* const memory = memArea->addr;
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;
	Node* nodes = parser->nodes;
	size_t buffer_i = *i;
	size_t buffer_j = *j;
	// rough allocation

	while(tokens[buffer_i].subtype != TokenSubtype_SEMICOLON
	   && buffer_i < parser->lexer->tokens.count - 1) {
		buffer_i += 1;
		buffer_j += 1;
	}

	if(parser_allocate_chunk(
		buffer_j + 1,
		parser)
	== false)
		return -1;

	buffer_i = *i;
	buffer_j = *j;
	// if the current scope has at least one parameter allocator->address[count_parenthesis_nest] is set to 1
	size_t count_parenthesis_nest = 0;	

	if(parser_is_R_left_parenthesis(&tokens[buffer_i]))
		goto R_LPARENTHESIS_SKIP_PARAMETER;

	do {
		size_t j_lock;
TYPE:
		// lock alone
		if(parse_type_lock_left(
			&buffer_i,
			&buffer_j,
			parent,
			parser)
		== false)
			return false;

		if(tokens[buffer_i].subtype != TokenSubtype_LPARENTHESIS) {
			type_bind_child_token(
				NodeTypeChildType_LOCK,
				(NodeSubtype) NodeSubtypeChild_NO,
				&tokens[buffer_i],
				&parent,
				&nodes[buffer_j]);
			j_lock = buffer_j;
			buffer_i += 1;
			buffer_j += 1;
			memory[count_parenthesis_nest] = 1;

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
			// handle R left parenthesis at the first nesting level like in :(a :())
			|| (count_parenthesis_nest == 1
			 && !parser_is_parenthesis(&tokens[buffer_i - 1])))
				goto R_LPARENTHESIS_SKIP_PARAMETER;

			if(!parser_is_key(&tokens[buffer_i]))
				return false;

			type_bind_child_token(
				NodeTypeChildType_LOCK,
				(NodeSubtype) NodeSubtypeChildTypeScoped_PARAMETER,
				&tokens[buffer_i],
				&parent,
				&nodes[buffer_j]);
			buffer_i += 1;
			buffer_j += 1;
R_LPARENTHESIS_SKIP_PARAMETER:
			type_bind_child_token(
				NodeTypeChildType_LOCK,
				(NodeSubtype) NodeSubtypeChildTypeScoped_RETURN_NONE,
				NULL,
				&parent,
				&nodes[buffer_j]);
			j_lock = buffer_j;
			buffer_i += 1;
			buffer_j += 1;
			memory[count_parenthesis_nest] = 1;
			count_parenthesis_nest += 1;
			memory[count_parenthesis_nest] = 0;
		}

		if(tokens[buffer_i].subtype == TokenSubtype_LPARENTHESIS) {
			nodes[j_lock].subtype = NodeSubtypeChildTypeScoped_RETURN_LOCK;
// LPARENTHESIS:
			// handle nested empty parenthesis like in :(())
			if(tokens[buffer_i - 1].subtype == TokenSubtype_LPARENTHESIS)
				return 0;

			buffer_i += 1;
			count_parenthesis_nest += 1;
			memory[count_parenthesis_nest] = 0;

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

			memory[count_parenthesis_nest] = 1;

			if(parser_is_key(&tokens[buffer_i])) {
				// ignore parameter after the first nesting level
				if(count_parenthesis_nest > 1) {
					buffer_i += 1;
					goto TYPE;
				}

				type_bind_child_token(
					NodeTypeChildType_LOCK,
					(NodeSubtype) NodeSubtypeChildTypeScoped_PARAMETER,
					&tokens[buffer_i],
					&parent,
					&nodes[buffer_j]);
				buffer_i += 1;
				buffer_j += 1;
				goto TYPE;
			} else if(count_parenthesis_nest > 1
			       && parser_is_lock(&tokens[buffer_i])) {
				goto TYPE;
			} else {
				// a lock must succeed a key at the first nesting level
				return 0;
			}
		} else if(tokens[buffer_i].subtype == TokenSubtype_RPARENTHESIS) {
RPARENTHESIS:
			if(memory[count_parenthesis_nest] == 0) {
				type_bind_child_token(
					NodeTypeChildType_LOCK,
					(NodeSubtype) NodeSubtypeChildTypeScoped_PARAMETER_NONE,
					NULL,
					&parent,
					&nodes[buffer_j]);
				buffer_j += 1;
			}

			do {
				buffer_i += 1;
				count_parenthesis_nest -= 1;
			} while(tokens[buffer_i].subtype == TokenSubtype_RPARENTHESIS);

			if(tokens[buffer_i].subtype == TokenSubtype_COMMA)
				goto COMMA;
		} else if(parser_is_key(&tokens[buffer_i])) {
				// keep ignoring parameter after the first nesting level
				if(count_parenthesis_nest > 1) {
					buffer_i += 1;
					goto TYPE;
				}
				// a parameter after an R left parenthesis like in :(a :b)
				type_bind_child_token(
					NodeTypeChildType_LOCK,
					(NodeSubtype) NodeSubtypeChildTypeScoped_PARAMETER,
					&tokens[buffer_i],
					&parent,
					&nodes[buffer_j]);
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
size_t* i,
size_t* j,
Node* parent,
Parser* parser) {
	// just parse literals for the moment, expressions later
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;
	size_t buffer_i = *i;
	size_t buffer_j = *j;

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
size_t* i,
size_t* j,
MemoryArea* memArea,
Parser* parser) {
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;
	size_t buffer_i = *i;
	size_t buffer_j = *j;
	Node* nodes = parser->nodes;
	NodeSubtype subtype = NodeSubtype_NO;
	size_t i_qualifier = buffer_i;

	while(parser_is_qualifier(&tokens[buffer_i])) buffer_i += 1;

	if(tokens[buffer_i].type != TokenType_COMMAND)
		return 0;

	subtype |= token_subtype_command_to_subtype(tokens[buffer_i].subtype);
	buffer_i += 1;

	if(tokens[buffer_i].type != TokenType_IDENTIFIER)
		return 0;

	nodes[buffer_j] = (Node) {
		.type = NodeType_IDENTIFICATION,
		.subtype = subtype,
		.token = &tokens[buffer_i],
		.child1 = NULL,
		.child2 = NULL};
	buffer_i += 1;
	buffer_j += 1;

	while(parser_is_qualifier(&tokens[i_qualifier])) {
		if(parser_allocate_chunk(
			buffer_j + 1,
			parser)
		== false)
			return -1;

		nodes[buffer_j] = (Node) {
			.type = NodeType_QUALIFIER,
			.subtype = tokens[i_qualifier].subtype,
			.token = &tokens[i_qualifier],
			.child1 = NULL,
			.child2 = NULL};
		nodes[buffer_j - 1].child1 = &nodes[buffer_j];
		i_qualifier += 1;
		buffer_j += 1;
	}
	// add the type as child nodes in `.child1`
	switch(if_type_create_nodes(
		&buffer_i,
		&buffer_j,
		&nodes[buffer_j - 1],
		memArea,
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
		parser->nodes[*j].subtype |= NodeSubtypeIdentificationBitType_DECLARATION;
		break;
	case 1: // initialization case
		parser->nodes[*j].subtype |= NodeSubtypeIdentificationBitType_INITIALIZATION;
		break;
	}

	*i = buffer_i;
	*j = buffer_j;
	return 1;
}

