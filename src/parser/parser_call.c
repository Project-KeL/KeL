#include <assert.h>
#include <string.h>
#include "parser_allocator.h"
#include "parser_call.h"
#include "parser_utils.h"
#include <stdio.h>

/*
 * "CPL" stands for "compile-time parameterized label"
*/

static bool is_type_match(
const Node* node1,
const Node* node2) {
	const Node* type1 = node1->child1;
	const Node* type2 = node2->child1;

	if((node1->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_SCOPED)
	!= (node2->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_SCOPED))
		return false;

	while(true) {
		if(type1->type != NodeTypeChildType_LOCK
		|| type2->type != NodeTypeChildType_LOCK)
			break;

		if(type1->subtype != type2->subtype)
			return false;

		type1 += 1;
		type2 += 1;
	}

	return type1->type != NodeTypeChildType_LOCK
	    && type2->type != NodeTypeChildType_LOCK;
}

static bool is_type_match_scope_file(
const Node* parameterized_label,
const Node* node) {	
}

static bool is_type_match_parameter(
const Node* parameterized_label,
const Node* node) {	
}

static bool is_type_match_scope_local(
const Node* parameterized_label,
const Node* declaration_node,
size_t count_parameter,
const Parser* parser,
const Node** node_declaration) {
	const char* code = parser->lexer->source->content;
	const Node* node = (Node*) parser->nodes.top;
	const Node* node_local = (Node*) parameterized_label->child2 + 1;
	// search the declaration of `node`
	while(node_local != node) {
		if(node_local->type == NodeType_IDENTIFICATION
		&& parser_is_token_L_match(
			code,
			node_local->token,
			node->token)
		== true)
			goto CHECK;

		node_local += 1;
	}

	return false;
CHECK:
	*node_declaration = node_local;
	return is_type_match(
		node_local->child1,
		declaration_node->child1);
}

static int if_parameters_create_nodes(
size_t* i,
const Node* parameterized_label,
const Node* declaration_node,
Parser* parser) {
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;
	size_t buffer_i = *i;
	size_t count_parameter = 0;
	const Node* node_declaration;

	if(!parser_allocator(parser))
		return -1;
	// return type
	declaration_node = declaration_node->child1;

	if(declaration_node->type == NodeTypeChildType_LOCK
	&& declaration_node->subtype == NodeSubtypeChildTypeScoped_RETURN_NONE) {
	}

	while(true) {
		if(!parser_allocator(parser))
			return -1;

		if(is_type_match_scope_local(
			parameterized_label,
			declaration_node,
			count_parameter,
			parser,
			&node_declaration)
		== true) {
			*((Node*) parser->nodes.top) = (Node) {
				.is_child = true,
				.type = NodeTypeChildCall_ARGUMENT};
			((Node*) parser->nodes.previous)->child = (Node*) parser->nodes.top;
		} else {
			return -1;
		}

		if(tokens[buffer_i].type != TokenSubtype_COMMA)
			return -1;

		count_parameter += 1;
		buffer_i += 1;
	}

	*i = buffer_i;
}

int if_call_create_nodes(
size_t* i,
const Node* parameterized_label,
Parser* parser) {
	return 0;
	const char* code = parser->lexer->source->content;
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;
	size_t buffer_i = *i;
	const MemoryChainLink* declaration_link;
	const Node* declaration_node = parser_allocator_start_declaration(
		parser,
		&declaration_link);

	if(declaration_node == NULL)
		return 0;

	while(parser_allocator_continue_declaration(
		parser,
		declaration_node)
	== true) {
		if(declaration_node->type == NodeType_NO)
			break;

		if(strncmp(
			code + declaration_node->token->L_start,
			code + tokens[buffer_i].L_start,
			declaration_node->token->L_end - declaration_node->token->L_start)
		== 0)
			goto FOUND;

		if(parser_allocator_next(
			parser,
			&declaration_link,
			&declaration_node)
		== false)
			break;
	}

	return 0;
FOUND:
	// get the first parameter
	buffer_i += 1;
	TokenSubtype command;
	bool right_parenthesis = false;
	
	if(parser_is_command(tokens + buffer_i)) {
		command = tokens[buffer_i].subtype;
		buffer_i += 1;

		if(tokens[buffer_i].subtype == TokenSubtype_LPARENTHESIS)
			goto LPARENTHESIS;
	} else if(tokens[buffer_i].subtype == TokenSubtype_LPARENTHESIS) {
		// this syntax is valid only for the `@` command
		if((declaration_node->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_COMMAND)
		!= NodeSubtypeIdentificationBitCommand_AT)
			return -1;

		command = TokenSubtype_AT;
LPARENTHESIS:
		right_parenthesis = true;
		buffer_i += 1;
	} else {
		command = TokenSubtype_HASH;
	}

	if(!parser_allocator(parser))
		return -1;

	*((Node*) parser->nodes.top) = (Node) {
		.is_child = false,
		.type = NodeType_CALL};

	switch(declaration_node->subtype & MASK_BIT_NODE_SUBTYPE_CALL_TIME) {
		case NodeSubtypeCallBitTime_COMPILE:
			switch(if_parameters_create_nodes(
				&buffer_i,
				parameterized_label,
				declaration_node,
				parser)) {
					case -1: return -1;
					case 0: return 0;
					case 1: /* fall through */ break;
				} break;
		default: return 0;
	}

	*i = buffer_i + 1;
	return 1;
}
