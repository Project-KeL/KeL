#include <assert.h>
#include <string.h>
#include "parser_allocator.h"
#include "parser_call.h"
#include "parser_introduction.h"
#include "parser_type.h"
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
	if((type1->subtype & MASK_BIT_NODE_SUBTYPE_INTRODUCTION_SCOPED)
	!= (type2->subtype & MASK_BIT_NODE_SUBTYPE_INTRODUCTION_SCOPED))
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

		type1 = type1->Introduction.type;
		type2 = type2->Introduction.type;
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
const Node* node_PAL_scope,
const MemoryChainLink* link_PAL_scope,
const Node* file_node,
const Parser* parser,
const Node** node_declaration) {
	const char* code = parser->lexer->source->content;
	const Node* node = (Node*) parser->nodes.top;
	// search the declaration of `node`
	while(node_PAL_scope != node) {
// printf("HERE: %d,  %d, %d\n", node_PAL_scope == link_PAL_scope->memArea.addr, node_PAL_scope->is_child, node_PAL_scope->type);
		parser_allocator_next(
			parser,
			&link_PAL_scope,
			&node_PAL_scope);

		if(node_PAL_scope->type == NodeType_INTRODUCTION
		&& parser_is_code_token_match(
			code,
			node_PAL_scope->child->token,
			file_node->token)
		== true)
			goto CHECK;
	}

	return false;
CHECK:
	*node_declaration = node_PAL_scope;
	return is_type_match(
		code,
		node_PAL_scope->child,
		file_node);
}

static bool call_child_bind_token(
NodeTypeChildCall type,
NodeSubtypeChild subtype,
const Token* token,
Parser* parser) {
	if(!parser_allocator(parser))
		return false;

	*((Node*) parser->nodes.top) = (Node) {
		.type = type,
		.subtype = subtype,
		.token = token,
		.ChildCall = {.next = NULL}};
	((Node*) parser->nodes.previous)->child = (Node*) parser->nodes.top;
	return true;
}

static int if_arguments_create_nodes(
bool right_parenthesis,
size_t* i,
const MemoryChainLink* link_PAL_scope,
const Node* node_PAL_scope,
uint64_t count_parameter,
const Node* file_node,
Parser* parser) {
	return 0;
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;
	size_t buffer_i = *i;
	size_t count_argument = 0;
	const Node* node_declaration = NULL;
	// skip the return type
	file_node = file_node->Introduction.type->Type.next;

	if((right_parenthesis
	 && tokens[buffer_i].subtype == TokenSubtype_RPARENTHESIS)
	|| (!right_parenthesis
	 && tokens[buffer_i].subtype == TokenSubtype_SEMICOLON)) {
		if(call_child_bind_token(
			NodeTypeChildCall_ARGUMENT_NONE,
			(NodeSubtypeChild) NodeSubtypeChildCall_NO,
			NULL,
			parser)
		== false)
			return -1;

		buffer_i += 1;
	} else {
		do {
			// skip the parameter
			file_node = file_node->child;

			if(is_type_match_scope_local(
				node_PAL_scope,
				link_PAL_scope,
				file_node,
				parser,
				&node_declaration)
			== true) {
				if(call_child_bind_token(
					NodeTypeChildCall_ARGUMENT,
					(NodeSubtypeChild) NodeSubtypeChildCall_NO,
					tokens + buffer_i,
					parser)
				== false)
					return -1;
			} else {
				return -1;
			}

			count_argument += 1;
			buffer_i += 1;

			if(tokens[buffer_i].subtype == TokenSubtype_COMMA) {
				buffer_i += 1;
			} else if((right_parenthesis
			 && tokens[buffer_i].subtype == TokenSubtype_RPARENTHESIS)
			|| (!right_parenthesis
			 && tokens[buffer_i].subtype == TokenSubtype_SEMICOLON)) {
				break;
			} else {
				return -1;
			}
		} while(true);
	
		if(count_argument != count_parameter)
			return -1;
	}

	*i = buffer_i + (right_parenthesis ? 1 : 0);
	return 1;
}

int if_call_create_nodes(
size_t* i,
const MemoryChainLink* link_PAL,
const Node* node_PAL,
Node** node_call_last,
Parser* parser) {
	assert(i != NULL);
	assert(node_PAL != NULL);
	assert(link_PAL != NULL);
	assert(node_call_last != NULL);
	assert(parser != NULL);

	assert(node_PAL->is_child == false);
	assert(node_PAL->type == NodeType_INTRODUCTION);
	assert(node_PAL->Introduction.initialization != NULL);
	assert(node_PAL->Introduction.initialization->type == NodeType_SCOPE_START);
	// the following assertion is only valid when `CHUNK` is `1`
	assert(link_PAL->memArea.addr == node_PAL->Introduction.initialization);

	const char* code = parser->lexer->source->content;
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;
	size_t buffer_i = *i;
	uint64_t count_parameter = 0;
	// go to the body of the scope
	node_PAL = node_PAL->Introduction.initialization;
	// get the first instruction after the scope
	parser_allocator_next(
		parser,
		&link_PAL,
		&node_PAL);

	if(tokens[buffer_i].type != TokenType_L)
		return 0;
	// search a match with an identifier at file scope
	const MemoryChainLink* file_link;
	const Node* file_node = parser_allocator_start_file_node(
		parser,
		&file_link);

	if(file_node == NULL)
		return 0;

	while(parser_allocator_continue_file_node(
		parser,
		file_node)
	== true) {
		bool found = false;
		count_parameter = 0;

		if(parser_is_code_token_match(
			code,
			tokens + buffer_i,
			file_node->token)
		== true)
			found = true;
		// count the parameters
		const Node* buffer_file_node = file_node;

		do {
			parser_allocator_next_link(
				buffer_file_node,
				&file_link);
			buffer_file_node = buffer_file_node->child;

			if(buffer_file_node->subtype == NodeSubtypeChildTypeScoped_PARAMETER)
				count_parameter += 1;
		} while(buffer_file_node->child != NULL);

		if(found)
			goto FOUND;

		if(parser_allocator_next(
			parser,
			&file_link,
			&file_node)
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
		.Call = {
			.PAL = NULL,
			.arguments = NULL}};
	*node_call_last = (Node*) parser->nodes.top;
	// get the return type
	buffer_i += 1;

	if(parser_is_lock(tokens + buffer_i)) {
		// compare with the expected return type
		if(file_node->Introduction.type->type == NodeTypeChildType_LOCK
		&& file_node->Introduction.type->subtype == NodeSubtypeChild_NO
		&& !parser_is_code_token_match(
			code,
			tokens + buffer_i,
			file_node->Introduction.type->token)
		== false)
			goto RESTORE;

		if(call_child_bind_token(
			NodeTypeChildCall_RETURN_TYPE,
			(NodeSubtypeChild) NodeSubtypeChildCall_NO,
			tokens + buffer_i,
			parser)
		== false)
			return -1;

		buffer_i += 1;
	} else {
		if(call_child_bind_token(
			NodeTypeChildCall_RETURN_UNKNOWN,
			(NodeSubtypeChild) NodeSubtypeChildCall_NO,
			NULL,
			parser)
		== false)
			return -1;
		// `buffer_i` is well set for the next step
	}
	// calling syntax
	NodeSubtypeIntroductionBitCommand command;
	bool right_parenthesis = false;

	if(parser_is_command(tokens + buffer_i)) {
		command = parser_identifier_token_subtype_TO_node_subtype_introduction_bit_command(tokens[buffer_i].subtype);
		buffer_i += 1;

		if(tokens[buffer_i].subtype == TokenSubtype_LPARENTHESIS)
			goto LPARENTHESIS;
	} else if(tokens[buffer_i].subtype == TokenSubtype_LPARENTHESIS) {
		// this syntax is valid only for the `@` command
		if((file_node->subtype & MASK_BIT_NODE_SUBTYPE_INTRODUCTION_COMMAND)
		!= NodeSubtypeIntroductionBitCommand_AT)
			return -1;

		command = NodeSubtypeIntroductionBitCommand_AT;
LPARENTHESIS:
		right_parenthesis = true;
		buffer_i += 1;
	} else {
		// it is a CPL by default
		command = NodeSubtypeIntroductionBitCommand_HASH;
	}
	// compare with the expected command at the declaration
	if(command != (file_node->subtype & MASK_BIT_NODE_SUBTYPE_INTRODUCTION_COMMAND))
		goto RESTORE;
	// process arguments
	switch(if_arguments_create_nodes(
		right_parenthesis,
		&buffer_i,
		link_PAL,
		node_PAL,
		count_parameter,
		file_node,
		parser)) {
	case -1: return -1;
	case 0: goto RESTORE;
	case 1: /* fall through */ break;
	}

	if(call_child_bind_token(
		NodeTypeChildCall_NO,
		(NodeSubtypeChild) NodeSubtypeChildCall_NO,
		NULL,
		parser)
	== false)
		return -1;

	*i = buffer_i;
	return 1;
RESTORE:
	memory_chain_state_restore(
		&parser->nodes,
		&memChain_state);
	return 0;
}
