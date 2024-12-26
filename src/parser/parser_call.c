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
		type1 = parser_type_get_tail(type1);
		type2 = parser_type_get_tail(type2);
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
	file_node = parser_type_get_tail(
		parser_introduction_get_type(file_node));

	for(;
	count_argument != 0;
	count_argument -= 1) {
		file_node = parser_type_get_tail(file_node);
	}

	assert(file_node != NULL);
	assert(file_node->type != NodeType_NO);

	return is_type_match(
		code,
		file_node,
		parser_introduction_get_type(node_PAL_scope));
}

static bool call_child_bind_token(
NodeTypeChildCall type,
const Token* token,
Parser* parser) {
	if(!parser_allocator(parser))
		return false;

	*((Node*) parser->nodes.top) = (Node) {
		.type = type,
		.subtype = (NodeSubtype) NodeSubtypeChildCall_NO,
		.token = token,
		.nodes = {
			[NODE_INDEX_CALL_PAL] = NULL,
			[NODE_INDEX_TAIL] = NULL}};
	((Node*) parser->nodes.previous)->nodes[NODE_INDEX_TAIL] = (Node*) parser->nodes.previous;
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
		assert(parser_introduction_get_initialization(node_PAL_scope) != NULL);
		assert(parser_introduction_get_initialization(node_PAL_scope)->type == NodeType_SCOPE_START);
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
			const Node* buffer_file_node = parser_introduction_get_type(file_node);
			buffer_file_node = parser_type_get_tail(buffer_file_node);

			while(buffer_file_node != NULL) {
				if(buffer_file_node->subtype == NodeSubtypeChildTypeScoped_PARAMETER)
					count_parameter += 1;

				buffer_file_node = parser_type_get_tail(buffer_file_node);
			}

			goto FOUND;
		}
	} while(parser_allocator_next(
		parser,
		&file_link,
		&file_node));

	return 0;
FOUND:
	return 0;
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
		.nodes = {
			[NODE_INDEX_CALL_PAL] = NULL,
			[NODE_INDEX_TAIL] = NULL}};
	*node_call_last = (Node*) parser->nodes.top;
	// get the return type
	buffer_i += 1;

	if(parser_is_lock(tokens + buffer_i)) {
		if(!parser_is_lock(parser_introduction_get_type(file_node)->token))
			goto RETURN_0;
		// compare with the expected return type
		if(parser_is_code_token_match(
			code,
			tokens + buffer_i,
			parser_introduction_get_type(file_node)->token)
		== false)
			goto RETURN_0;

		if(call_child_bind_token(
			NodeTypeChildCall_RETURN_TYPE,
			tokens + buffer_i,
			parser)
		== false)
			return -1;

		buffer_i += 1;
	} else {
		if(call_child_bind_token(
			// unknown because of the type deducing
			NodeTypeChildCall_RETURN_UNKNOWN,
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
			goto RETURN_0;

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
		goto RETURN_0;
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
	case 0: goto RETURN_0;
	case 1: /* fall through */ break;
	}

	if(call_child_bind_token(
		NodeTypeChildCall_NO,
		NULL,
		parser)
	== false)
		return -1;

	*i = buffer_i;
	return 1;
RETURN_0:
	memory_chain_state_restore(
		&parser->nodes,
		&memChain_state);
	return 0;
}

bool parser_is_call(const Node* node) {
	return node != NULL
        && !node->is_child
	    && node->type == NodeType_CALL;
}

bool parser_call_is_time_compile(const Node* call) {
	assert(parser_is_call(call));

	return (call->subtype & MASK_BIT_NODE_SUBTYPE_CALL_TIME)
	    == NodeSubtypeCallBitTime_COMPILE;
}

bool parser_call_is_time_binary(const Node* call) {
	assert(parser_is_call(call));

	return (call->subtype & MASK_BIT_NODE_SUBTYPE_CALL_TIME)
	    == NodeSubtypeCallBitTime_BINARY;
}

bool parser_call_is_time_run(const Node* call) {
	assert(parser_is_call(call));

	return (call->subtype & MASK_BIT_NODE_SUBTYPE_CALL_TIME)
	    == NodeSubtypeCallBitTime_RUN;
}

bool parser_call_is_return(const Node* call) {
	assert(parser_is_call(call));

	return (call->subtype & MASK_BIT_NODE_SUBTYPE_CALL_RETURN)
	    == NodeSubtypeCallBitReturn_TRUE;
}

bool parser_call_is_return_deduce(const Node* call) {
	assert(parser_is_call(call));

	return (call->subtype & MASK_BIT_NODE_SUBTYPE_CALL_RETURN_DEDUCE)
		== NodeSubtypeCallBitReturnDeduce_TRUE;
}

[[deprecated]] void parser_call_set_tail(
Node* call,
Node* tail) {
	assert(parser_is_call(call));

	call->nodes[NODE_INDEX_TAIL] = tail;
}

void parser_call_set_PAL(
Node* call,
Node* PAL) {
	assert(parser_is_call(call));
#ifndef NDEBUG
	if(PAL != NULL)
		assert(parser_introduction_is_PAL(PAL));
#endif
	call->nodes[NODE_INDEX_CALL_PAL] = PAL;
}

[[deprecated]] const Node* parser_call_get_tail(const Node* call) {
	assert(parser_is_call(call));

	return call->nodes[NODE_INDEX_TAIL];
}

const Node* parser_call_get_PAL(const Node* call) {
	assert(parser_is_call(call));

	return call->nodes[NODE_INDEX_CALL_PAL];
}
