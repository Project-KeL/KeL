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
	while(type1->type != NodeType_NO
	   && type2->type != NodeType_NO) {
		if(type1->type == NodeTypeChildType_LOCK) {
			if(type2->type != NodeTypeChildType_LOCK)
				return false;

			if(type1->subtype == NodeSubtypeChildTypeScoped_RETURN_NONE) {
				if(type2->subtype != NodeSubtypeChildTypeScoped_RETURN_NONE)
					return false;
				// no token to check
				goto NEXT;
			} else if(type1->subtype == NodeSubtypeChildTypeScoped_PARAMETER_NONE) {
				if(type2->subtype != NodeSubtypeChildTypeScoped_PARAMETER_NONE)
					return false;
			}

			if(parser_is_code_token_match(
				code,
				type1->token,
				type2->token)
			== false)
				return false;
		}
NEXT:
		type1 = type1->Type.next;
		type2 = type2->Type.next;
	}

	return true; 
}

static bool is_type_match_scope_file(
const Node* node_parameterized_label,
const Node* node) {	
	return false;
}

static bool is_match_parameter(
size_t count_argument,
const MemoryChainLink* link_PAL,
const Node* node_PAL,
const Parser* parser,
const Node* file_node,
const Node** node_introduction) {
	return false;
}

static bool is_match_scope_local(
size_t count_argument,
const MemoryChainLink* link_PAL_scope,
const Node* node_PAL_scope,
const Parser* parser,
const Node* file_node,
const Node** node_introduction) {
	const char* code = parser->lexer->source->content;
	const Node* argument = (Node*) parser->nodes.top;
	// skip until the first instruction after the scope
	while(parser_allocator_next(
		parser,
		&link_PAL_scope,
		&node_PAL_scope)) {
			if(!node_PAL_scope->is_child
			&& node_PAL_scope->type == NodeType_SCOPE_START)
				break;
	}
	// search the declaration of `node` in the local scope
	while(node_PAL_scope != argument) {
		parser_allocator_next(
			parser,
			&link_PAL_scope,
			&node_PAL_scope);

		if(node_PAL_scope->is_child == false
		&& node_PAL_scope->type == NodeType_INTRODUCTION
		&& parser_is_code_token_match(
			code,
			node_PAL_scope->token,
			file_node->token)
		== true)
			goto CHECK_TYPE;
	}

	return false;
CHECK_TYPE:
	*node_introduction = node_PAL_scope;
	file_node = file_node->Introduction.type->Type.next;

	for(;
	count_argument != 0;
	count_argument -= 1) {
		file_node = file_node->Type.next;
	}

	assert(file_node != NULL);
	assert(file_node->type != NodeType_NO);

	return is_type_match(
		code,
		file_node,
		node_PAL_scope->Introduction.type);
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
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;
	size_t buffer_i = *i;
	size_t count_argument = 0;
	// the introduction of the variable we are looking for
	const Node* node_introduction = NULL;
	
	if((right_parenthesis
	 && tokens[buffer_i].subtype == TokenSubtype_RPARENTHESIS)
	|| (!right_parenthesis
	 && tokens[buffer_i].subtype == TokenSubtype_SEMICOLON)) {
		// no argument case
		if(call_child_bind_token(
			NodeTypeChildCall_ARGUMENT_NONE,
			(NodeSubtypeChild) NodeSubtypeChildCall_NO,
			NULL,
			parser)
		== false)
			return -1;

		buffer_i += 1;
	} else {
		// argument case
		while(true) {
			if(is_match_parameter(
				count_argument,
				link_PAL_scope,
				node_PAL_scope,
				parser,
				file_node,
				&node_introduction)
			== true) {
			} else if(is_match_scope_local(
				count_argument,
				link_PAL_scope,
				node_PAL_scope,
				parser,
				file_node,
				&node_introduction)
			== true) {
				if(call_child_bind_token(
					NodeTypeChildCall_ARGUMENT,
					(NodeSubtypeChild) NodeSubtypeChildCall_NO,
					node_introduction->token,
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
		}
	
		if(count_argument != count_parameter)
			return -1;
	}

	*i = buffer_i + (right_parenthesis ? 1 : 0);
	return 1;
}

int if_call_create_nodes(
size_t* i,
const MemoryChainLink* link_PAL_scope,
const Node* node_PAL_scope,
Node** node_call_last,
Parser* parser) {
	assert(i != NULL);
	assert(node_call_last != NULL);
	assert(parser != NULL);
#ifndef NDEBUG
	if(link_PAL_scope != NULL
	&& node_PAL_scope != NULL) {
		assert(node_PAL_scope->is_child == false);
		assert(node_PAL_scope->type == NodeType_INTRODUCTION);
		assert(node_PAL_scope->Introduction.initialization != NULL);
		assert(node_PAL_scope->Introduction.initialization->type == NodeType_SCOPE_START);
		assert((Node*) link_PAL_scope->memArea.addr <= node_PAL_scope
			&& node_PAL_scope < (Node*) link_PAL_scope->memArea.addr + link_PAL_scope->memArea.count);
	}
#endif
	const char* code = parser->lexer->source->content;
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;
	size_t buffer_i = *i;
	uint64_t count_parameter = 0;
	
	if(tokens[buffer_i].type != TokenType_L)
		return 0;
	// search a match with an identifier at file scope
	const MemoryChainLink* file_link = NULL;
	const Node* file_node = parser_allocator_start_file_node(
		parser,
		&file_link);

	if(file_node == NULL)
		return 0;

	do {
		count_parameter = 0;

		if(parser_is_code_token_match(
			code,
			tokens + buffer_i,
			file_node->token)
		== true) {
			// count the parameters
			assert(file_node->type == NodeType_INTRODUCTION);

			const Node* buffer_file_node = file_node->Introduction.type;

			do {
				buffer_file_node = buffer_file_node->Type.next;
				
				assert(buffer_file_node->is_child);

				if(buffer_file_node->subtype == NodeSubtypeChildTypeScoped_PARAMETER)
					count_parameter += 1;
			} while(buffer_file_node->type != NodeType_NO);

			goto FOUND;
		}
	} while(parser_allocator_next(
		parser,
		&file_link,
		&file_node));

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
		if(!parser_is_lock(file_node->Introduction.type->token))
			return 0;
		// compare with the expected return type
		if(parser_is_code_token_match(
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
			// unknown because of the type deducing
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
			return 0;

		command = NodeSubtypeIntroductionBitCommand_AT;
LPARENTHESIS:
		right_parenthesis = true;
		buffer_i += 1;
	} else {
		// it is a CPL by default
		command = NodeSubtypeIntroductionBitCommand_HASH;
		buffer_i += 1;
	}
	// compare with the expected command at file scope
	if(command != (file_node->subtype & MASK_BIT_NODE_SUBTYPE_INTRODUCTION_COMMAND))
		goto RESTORE;
	// process arguments
	switch(if_arguments_create_nodes(
		right_parenthesis,
		&buffer_i,
		link_PAL_scope,
		node_PAL_scope,
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

bool parser_is_valid_call(const Node* node) {
	assert(node->is_child == false);
	assert(node->type == NodeType_CALL);

	return true;
}

bool parser_call_is_time_compile(const Node* node) {
	return (node->subtype & MASK_BIT_NODE_SUBTYPE_CALL_TIME)
	    == NodeSubtypeCallBitTime_COMPILE;
}

bool parser_call_is_time_binary(const Node* node) {
	return (node->subtype & MASK_BIT_NODE_SUBTYPE_CALL_TIME)
	    == NodeSubtypeCallBitTime_BINARY;
}

bool parser_call_is_time_run(const Node* node) {
	return (node->subtype & MASK_BIT_NODE_SUBTYPE_CALL_TIME)
	    == NodeSubtypeCallBitTime_RUN;
}

bool parser_call_is_return(const Node* node) {
	return (node->subtype & MASK_BIT_NODE_SUBTYPE_CALL_RETURN)
	    == NodeSubtypeCallBitReturn_TRUE;
}

bool parser_call_is_return_deduce(const Node* node) {
	return (node->subtype & MASK_BIT_NODE_SUBTYPE_CALL_RETURN_DEDUCE)
		== NodeSubtypeCallBitReturnDeduce_TRUE;
}

Node* parser_call_get_PAL(Node* node) {
	assert(parser_is_valid_type(node));

	return node->Call.PAL;
}
