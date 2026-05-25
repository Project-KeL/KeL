#include "parser_expression.h"
#include "parser.h"
#include "parser_node.h"
#include "parser_utils.h"
#include "allocator.h"
#include <stdio.h>

bool if_EXP_create_operator(
size_t* i,
size_t* j,
MemoryStack* stack_operator,
MemoryStack* stack_buffer,
Parser* parser) {
	const Token* const tokens = parser->lexer->tokens.base;

	if(!parser_is_key(tokens + *i)
	&& !parser_is_literal(tokens + *i))
		return false;
	// shunting yard algorithm
	Operator operator = (Operator) {
		.type = NodeType_EXP,
		.precedence = 0,
		.count_arity = 0,
		.token = *i};
	memory_stack_push(
		(char*) &operator,
		stack_operator);
	size_t buffer_i = *i;
	size_t buffer_j = *j;

	while(!parser_is_instruction_exit(tokens + buffer_i)) {
		if(parser_is_literal_number(tokens + buffer_i)) {
			parser_create_leaf(
				NodeType_LIT_NUM,
				buffer_i,
				&buffer_j,
				stack_operator,
				parser);
			buffer_i += 1;
		} else if(parser_is_operator_algebraic(tokens + buffer_i)) {
			NodeType type;
			uint32_t precedence;

			if(parser_is_operator_ADD(tokens + buffer_i)) {
				type = NodeType_OP_ADD;
				precedence = 1;
			} else if(parser_is_operator_SUB(tokens + buffer_i)) {
				type = NodeType_OP_SUB;
				precedence = 1;
			} else if(parser_is_operator_MUL(tokens + buffer_i)) {
				type = NodeType_OP_MUL;
				precedence = 2;
			} else if(parser_is_operator_DIV(tokens + buffer_i)) {
					type = NodeType_OP_DIV;
					precedence = 2;
			} else
				return false;

			while(!memory_stack_is_empty(stack_buffer)) {
				Operator* top_operator = memory_stack_top_addr(stack_buffer);

				if(top_operator->type == NodeType_OP_LPARENTHESIS)
					break;

				if(top_operator->precedence < precedence)
					break;

				Operator pop_operator;
				memory_stack_pop(
					(char*) &pop_operator,
					stack_buffer);
				parser_create_operator(
					pop_operator.type,
					2,
					pop_operator.token,
					&buffer_j,
					stack_operator,
					parser);
			}

			Operator operator = (Operator) {
				.type = type,
				.precedence = precedence,
				.count_arity = 0,
				.token = buffer_i};
			memory_stack_push(
				(char*) &operator,
				stack_buffer);
			buffer_i += 1;
		} else if(parser_is_L_left_parenthesis(tokens + buffer_i)) {
			Operator operator = (Operator) {
				.type = NodeType_OP_LPARENTHESIS,
				.precedence = 0,
				.count_arity = 0,
				.token = buffer_i};
			memory_stack_push(
				(char*) &operator,
				stack_buffer);
			buffer_i += 1;
		} else if(parser_is_L_right_parenthesis(tokens + buffer_i)) {
			while(!memory_stack_is_empty(stack_buffer)
			   && ((Operator*) memory_stack_top_addr(stack_buffer))->type
			!= NodeType_OP_LPARENTHESIS) {
				Operator pop_operator;
				memory_stack_pop(
					(char*) &pop_operator,
					stack_buffer);
				parser_create_operator(
					pop_operator.type,
					2,
					pop_operator.token,
					&buffer_j,
					stack_operator,
					parser);

				if(memory_stack_is_empty(stack_buffer))
					return false;
			}
			// throw `(`
			memory_stack_discard(stack_buffer);
			buffer_i += 1;
		}
	}

	while(!memory_stack_is_empty(stack_buffer)) {
		Operator* top_operator = memory_stack_top_addr(stack_buffer);

		if(top_operator->type == NodeType_OP_LPARENTHESIS) // unclosed `(`
			return false;

		Operator pop_operator;
		memory_stack_pop(
			(char*) &pop_operator,
			stack_buffer);
		parser_create_operator(
			pop_operator.type,
			2,
			pop_operator.token,
			&buffer_j,
			stack_operator,
			parser);
	}
	// the arity of `EXP` must be `1`
	((Operator*) memory_stack_top_addr(stack_operator))->count_arity = 1;
	*i = buffer_i;
	*j = buffer_j;
	return true;
}
