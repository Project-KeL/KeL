#include <assert.h>
#include <string.h>
#include "parser_allocator.h"
#include "parser_call.h"
#include "parser_identifier.h"
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
const Node* node_parameterized_label,
const Node* node) {	
	return false;
}

static bool is_type_match_parameter(
const Node* node_parameterized_label,
const Node* node) {	
	return false;
}

static bool is_type_match_scope_local(
const Node* node_parameterized_label,
const MemoryChainLink* link_parameterized_label_scope,
const Node* declaration_node,
const Parser* parser,
const Node** node_declaration) {
	const char* code = parser->lexer->source->content;
	const Node* node = (Node*) parser->nodes.top;
	const Node* node_local = (Node*) node_parameterized_label->child2;
	// search the declaration of `node`
	while(node_local != node) {
		parser_allocator_next(
			parser,
			&link_parameterized_label_scope,
			&node_local);

		if(node_local->type == NodeType_IDENTIFICATION
		&& parser_is_code_token_match(
			code,
			node_local->token,
			declaration_node->token)
		== true)
			goto CHECK;
	}

	return false;
CHECK:
	*node_declaration = node_local;
	return is_type_match(
		node_local->child1,
		declaration_node->child1);
}

static int if_arguments_create_nodes(
size_t* i,
const Node* node_parameterized_label,
const MemoryChainLink* link_parameterized_label_scope,
uint64_t count_parameter,
const Node* declaration_node,
Parser* parser) {
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;
	size_t buffer_i = *i;
	size_t count_argument = 0;
	const Node* node_declaration = NULL;

	if(tokens[buffer_i].type == TokenType_NO) {
		return 0;
	} else if(tokens[buffer_i].subtype == TokenSubtype_RPARENTHESIS
	|| tokens[buffer_i].subtype == TokenSubtype_SEMICOLON) {
		if(!parser_allocator(parser))
			return -1;

		*((Node*) parser->nodes.top) = (Node) {
			.is_child = true,
			.type = NodeTypeChildCall_ARGUMENT_NONE};
		((Node*) parser->nodes.previous)->child = (Node*) parser->nodes.top;
		buffer_i += 1;
	} else {
		while(tokens[buffer_i].subtype != TokenSubtype_RPARENTHESIS
		   || tokens[buffer_i].subtype != TokenSubtype_SEMICOLON) {
			declaration_node = declaration_node->child1; // skip the parameter

			if(is_type_match_scope_local(
				node_parameterized_label,
				link_parameterized_label_scope,
				declaration_node,
				parser,
				&node_declaration)
			== true) {
				if(!parser_allocator(parser))
					return -1;

				*((Node*) parser->nodes.top) = (Node) {
					.is_child = true,
					.type = NodeTypeChildCall_ARGUMENT};
				((Node*) parser->nodes.previous)->child = (Node*) parser->nodes.top;
			} else {
				break;
			}

			if(tokens[buffer_i].type != TokenSubtype_COMMA)
				return -1;

			count_argument += 1;
			buffer_i += 1;
		}

		if(count_argument != count_parameter)
			return -1;
	}

	*i = buffer_i;
	return 1;
}

int if_call_create_nodes(
size_t* i,
const Node* node_parameterized_label,
const MemoryChainLink* link_parameterized_label_scope,
Node** node_call_last,
Parser* parser) {
	return 0;
	assert(i != NULL);
	assert(node_parameterized_label != NULL);
	assert(link_parameterized_label_scope != NULL);
	assert(node_call_last != NULL);
	assert(parser != NULL);

	assert(node_parameterized_label->child2 != NULL);
	assert(node_parameterized_label->child2->type == NodeType_SCOPE_START);
	assert(link_parameterized_label_scope->memArea.addr == node_parameterized_label->child2);

	const char* code = parser->lexer->source->content;
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;
	size_t buffer_i = *i;
	uint64_t count_parameter = 0;
	// search a match with an identifier at file scope
	const MemoryChainLink* declaration_link;
	const Node* declaration_node = parser_allocator_start_declaration(
		parser,
		&declaration_link);

	if(declaration_node == NULL)
		return 0;

	if(tokens[buffer_i].type != TokenType_L)
		return 0;

	while(parser_allocator_continue_declaration(
		parser,
		declaration_node)
	== true) {
		count_parameter = 0;
		bool found = false;

		if(declaration_node->type == NodeType_NO)
			goto NEXT;

		if(parser_is_code_token_match(
			code,
			tokens + buffer_i,
			declaration_node->token)
		== true)
			found = true;

		do {
			parser_allocator_next_link(
				declaration_node,
				&declaration_link);
			declaration_node = declaration_node->child1;

			if(declaration_node->subtype == NodeSubtypeChildTypeScoped_PARAMETER)
				count_parameter += 1;
		} while(declaration_node->child1 != NULL);

		if(found)
			goto FOUND;
NEXT:
		if(parser_allocator_next(
			parser,
			&declaration_link,
			&declaration_node)
		== false)
			break;
	}

	return 0;
FOUND:
	MemoryChainState memChain_state;
	initialize_memory_chain_state(&memChain_state);
	memory_chain_state_save(
		&parser->nodes,
		&memChain_state);

	if(!parser_allocator(parser))
		return -1;

	*((Node*) parser->nodes.top) = (Node) {
		.is_child = false,
		.type = NodeType_CALL,
		.child1 = NULL,
		.child2 = NULL};
	*node_call_last = (Node*) parser->nodes.top;
	// get the return type
	declaration_node = declaration_node->child1;
	buffer_i += 1;

	if(!parser_allocator(parser))
		return -1;

	if(parser_is_lock(tokens + buffer_i)) {
		*((Node*) parser->nodes.top) = (Node) {
			.is_child = true,
			.token = tokens + buffer_i,
			.type = NodeTypeChildCall_RETURN_TYPE};
		buffer_i += 1;
	} else {
		*((Node*) parser->nodes.top) = (Node) {
			.is_child = true,
			.token = NULL,
			.type = NodeTypeChildCall_RETURN_UNKNOWN};
		// `buffer_i` is well set for the next step
	}

	((Node*) parser->nodes.previous)->child1 = (Node*) parser->nodes.top;
	// calling syntax
	TokenSubtype command;
	bool right_parenthesis = false;

	if(parser_is_command(tokens + buffer_i)) {
		command = parser_identification_token_subtype_TO_node_subtype_identification_bit_command(tokens[buffer_i].subtype);
		buffer_i += 1;

		if(tokens[buffer_i].subtype == TokenSubtype_LPARENTHESIS)
			goto LPARENTHESIS;
	} else if(tokens[buffer_i].subtype == TokenSubtype_LPARENTHESIS) {
		// this syntax is valid only for the `@` command
		if((declaration_node->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_COMMAND)
		!= NodeSubtypeIdentificationBitCommand_AT)
			return -1;

		command = NodeSubtypeIdentificationBitCommand_AT;
LPARENTHESIS:
		right_parenthesis = true;
		buffer_i += 1;
	} else {
		// it is a CPL by default
		command = NodeSubtypeIdentificationBitCommand_HASH;
	}
	// compare with the expected command at the declaration
	if(command != (declaration_node->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_COMMAND)) {
		memory_chain_state_restore(
			&parser->nodes,
			&memChain_state);
		return 0;
	}
	// process arguments
	switch(if_arguments_create_nodes(
		&buffer_i,
		node_parameterized_label,
		link_parameterized_label_scope,
		count_parameter,
		declaration_node,
		parser)) {
	case -1: return -1;
	case 0: return 0;
	case 1: /* fall through */ break;
	}

	if(!parser_allocator(parser))
		return -1;

	*((Node*) parser->nodes.top) = (Node) {
		.is_child = true,
		.type = NodeTypeChild_NO};
	((Node*) parser->nodes.previous)->child1 = (Node*) parser->nodes.top;

	*i = buffer_i + right_parenthesis ? 1 : 0;
	return 1;
}
