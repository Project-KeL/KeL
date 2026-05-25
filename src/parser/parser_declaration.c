#include <assert.h>
#include <stddef.h>
#include "allocator.h"
#include "parser_declaration.h"
#include "lexer.h"
#include "parser_node.h"
#include "parser.h"
#include "parser_expression.h"
#include "parser_scope.h"
#include "parser_type.h"
#include "parser_utils.h"
#include <stdio.h>

static bool if_ID_create_leaf(
size_t* i,
size_t* j,
MemoryStack* stack_operator,
Parser* parser) {
	const Token* tokens = parser->lexer->tokens.base;

	if(!parser_is_ID(tokens + *i))
		return false;

	parser_create_leaf(
		NodeType_ID,
		*i,
		j,
		stack_operator,
		parser);
	*i += 1;
	return true;
}

static void if_INIT_create_operator(
NodeType DECL, // VAR or PAL
size_t* i,
size_t* j,
MemoryStack* stack_operator,
MemoryStack* stack_buffer,
Parser* parser) {
	assert(DECL == NodeType_DECL_VAR
	    || DECL == NodeType_DECL_PAL);

	Token* tokens = parser->lexer->tokens.base;
	size_t buffer_i = *i;

	if(parser_is_instruction_INIT_equal(tokens + *i))
		buffer_i += 1;

	size_t buffer_j = *j;

	if(DECL == NodeType_DECL_VAR) {
		MemoryStackState stack_state;
		initialize_memory_stack_state(&stack_state);
		memory_stack_state_save(
			stack_operator,
			&stack_state);

		Operator operator = (Operator) {
			.type = NodeType_INIT_VAR,
			.precedence = 0,
			.count_arity = 0,
			.token = buffer_i};
		memory_stack_push(
			(char*) &operator,
			stack_operator);

		if(if_EXP_create_operator(
			&buffer_i,
			&buffer_j,
			stack_operator,
			stack_buffer,
			parser)
		==false) {
			memory_stack_state_restore(
				stack_operator,
				&stack_state);
			return;
		}
		// pop the EXP
		Operator pop_operator;
		memory_stack_pop(
			(char*) &pop_operator,
			stack_operator);
		parser_create_operator(
			pop_operator.type,
			pop_operator.count_arity,
			pop_operator.token,
			&buffer_j,
			stack_operator,
			parser);
		// pop the INIT
		memory_stack_pop(
			(char*) &pop_operator,
			stack_operator);
		parser_create_operator(
			pop_operator.type,
			pop_operator.count_arity,
			pop_operator.token,
			&buffer_j,
			stack_operator,
			parser);
	} else if(DECL == NodeType_DECL_PAL) {
		if(parser_is_LSCOPE_start(tokens + buffer_i)) {
			Operator operator = (Operator) {
				.type = NodeType_INIT_PAL,
				.precedence = 0,
				.count_arity = 0,
				.token = buffer_i};
			memory_stack_push(
				(char*) &operator,
				stack_operator);
		}
	}

	*i = buffer_i;
	*j = buffer_j;
}

bool if_DECL_create_operator(
size_t* i,
size_t* j,
MemoryStack* stack_context,
MemoryStack* stack_operator,
MemoryStack* stack_buffer,
Parser* parser) {
	const Token* tokens = (const Token*) parser->lexer->tokens.base;
	size_t buffer_i = *i;

	if(tokens[buffer_i].type != TokenType_COM)
		return false;
	// `buffer_i` to look for an R parenthesis
	do {
		if(parser_is_instruction_exit(tokens + buffer_i))
			break;

		buffer_i += 1;
	} while(!parser_is_R_left_parenthesis(tokens + buffer_i));

	NodeType type_DECL = NodeType_NO;
	size_t buffer_j = *j;

	if(parser_is_R_left_parenthesis(tokens + buffer_i))
		type_DECL = NodeType_DECL_PAL;
	else
		type_DECL = NodeType_DECL_VAR;
	
	MemoryStackState stack_state;
	initialize_memory_stack_state(&stack_state);
	memory_stack_state_save(
		stack_operator,
		&stack_state);

	Operator operator = (Operator) {
		.type = type_DECL,
		.precedence = 0,
		.count_arity = 0,
		.token = *i};
	memory_stack_push(
		(char*) &operator,
		stack_operator);
	// `buffer_i` to look for an identifier
	buffer_i = *i + 1;

	if(if_ID_create_leaf(
		&buffer_i,
		&buffer_j,
		stack_operator,
		parser)
	== false)
		goto POP;
	//`buffer_i` to look for a type
	if(if_TYPE_create_operator(
		&buffer_i,
		&buffer_j,
		stack_operator,
		parser)
	== false)
		goto POP;
	// initialization
	if_INIT_create_operator(
		type_DECL,
		&buffer_i,
		&buffer_j,
		stack_operator,
		stack_buffer,
		parser);
	// `buffer_i` to look for an initialization (TODO)
	*i = buffer_i;
	*j = buffer_j;
	return true;
POP:
	memory_stack_state_restore(
		stack_operator,
		&stack_state);
	return false;
}
