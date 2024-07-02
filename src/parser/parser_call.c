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
const char* code,
const Node* type1,
const Node* type2) {
	// comparison of the type of a parameter
	// the parameters inside the type are ignored
	if((type1->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_SCOPED)
	!= (type2->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_SCOPED))
		return false;

	while(true) {
		if(type1->type == NodeType_NO
		|| type2->type == NodeType_NO)
			break;

		if(type1->subtype != type2->subtype
		|| parser_is_code_token_match(
			code,
			type1->token,
			type2->token)
		== false)
			return false;

		type1 = type1->child1;
		type2 = type2->child1;
	}

	return type1->type == type2->type;
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
			node_local->child1->token,
			declaration_node->token)
		== true)
			goto CHECK;
	}

	return false;
CHECK:
	*node_declaration = node_local;
	return is_type_match(
		code,
		node_local->child1,
		declaration_node);
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
	// skip the return type
	declaration_node = declaration_node->child1->child1;

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
					.token = tokens + buffer_i,
					.type = NodeTypeChildCall_ARGUMENT};
				((Node*) parser->nodes.previous)->child = (Node*) parser->nodes.top;
			} else {
				break;
			}

			count_argument += 1;
			buffer_i += 1;

			if(tokens[buffer_i].type != TokenSubtype_COMMA) {
				if(tokens[buffer_i].subtype == TokenSubtype_RPARENTHESIS
				|| tokens[buffer_i].subtype == TokenSubtype_SEMICOLON)
					break;

				return -1;
			}
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

	if(tokens[buffer_i].type != TokenType_L)
		return 0;
	// search a match with an identifier at file scope
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
		// count the parameters
		const Node* buffer_declaration_node = declaration_node;

		do {
			parser_allocator_next_link(
				buffer_declaration_node,
				&declaration_link);
			buffer_declaration_node = buffer_declaration_node->child1;

			if(buffer_declaration_node->subtype == NodeSubtypeChildTypeScoped_PARAMETER)
				count_parameter += 1;
		} while(buffer_declaration_node->child1 != NULL);

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
		.token = tokens + buffer_i,
		.child1 = NULL,
		.child2 = NULL};
	*node_call_last = (Node*) parser->nodes.top;
	// get the return type
	buffer_i += 1;

	if(!parser_allocator(parser))
		return -1;

	if(parser_is_lock(tokens + buffer_i)) {
		// compare with the expected return type
		if(declaration_node->child1->type == NodeTypeChildType_LOCK
		&& declaration_node->child1->subtype == NodeTypeChild_NO
		&& !parser_is_code_token_match(
			code,
			tokens + buffer_i,
			declaration_node->child1->token)
		== false)
			goto RESTORE;

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
	if(command != (declaration_node->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_COMMAND))
		goto RESTORE;
	// process arguments
	switch(if_arguments_create_nodes(
		&buffer_i,
		node_parameterized_label,
		link_parameterized_label_scope,
		count_parameter,
		declaration_node,
		parser)) {
	case -1: return -1;
	case 0: goto RESTORE;
	case 1: /* fall through */ break;
	}

	if(!parser_allocator(parser))
		return -1;

	*((Node*) parser->nodes.top) = (Node) {
		.is_child = true,
		.type = NodeTypeChild_NO};
	((Node*) parser->nodes.previous)->child1 = (Node*) parser->nodes.top;

	*i = buffer_i + (right_parenthesis ? 1 : 0);
	return 1;
RESTORE:
	memory_chain_state_restore(
		&parser->nodes,
		&memChain_state);
	return 0;
}
