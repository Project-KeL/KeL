#include "parser_type.h"
#include "allocator.h"
#include "lexer.h"
#include "parser.h"
#include "parser_node.h"
#include "parser_utils.h"
#include <stdio.h>

static bool if_TYPE_VAR_create_leaf(
size_t* i,
size_t* j,
MemoryStack* stack_operator,
Parser* parser) {
	const Token* const tokens = parser->lexer->tokens.base;

	if(tokens[*i].type != TokenType_R
	&& tokens[*i].type != TokenType_RSCOPE)
		return false;

	parser_create_leaf(
		NodeType_TYPE_VAR,
		*i,
		j,
		stack_operator,
		parser);
	*i += 1;
	return true;
}

static bool if_PARAM_create_leaf(
size_t* i,
size_t* j,
MemoryStack* stack_operator,
Parser* parser) {
	const Token* const tokens = parser->lexer->tokens.base;

	if(tokens[*i].type != TokenType_L)
		return false;

	parser_create_leaf(
		NodeType_PARAM,
		*i,
		j,
		stack_operator,
		parser);
	*i += 1;
	return true;
}

static bool if_GRP_RPARES_create_operator(
size_t* i,
size_t* j,
MemoryStack* stack_operator,
Parser* parser) {
	const Token* const tokens = parser->lexer->tokens.base;

	if(!parser_is_R_left_parenthesis(tokens + *i))
		return false;

	Operator operator = (Operator) {
		.type = NodeType_GRP_R_PARES,
		.precedence = 0,
		.count_arity = 0,
		.token = *i};
	memory_stack_push(
		(char*) &operator,
		stack_operator);
	size_t buffer_i = *i;
	size_t buffer_j = *j;
	buffer_i += 1;

	if(!parser_is_R_right_parenthesis(tokens + buffer_i)) {
		do {
			if(parser_is_instruction_exit(tokens + buffer_i))
				return false;

			if(if_PARAM_create_leaf(
				&buffer_i,
				&buffer_j,
				stack_operator,
				parser)
			== false)
				return false;

			if(if_TYPE_VAR_create_leaf(
				&buffer_i,
				&buffer_j,
				stack_operator,
				parser)
			== false)
				return false;

			if(!parser_is_PAL_comma(tokens + buffer_i)) {
				if(!parser_is_R_right_parenthesis(tokens + buffer_i))
					return false;
				else {
					break;
				}
			} else
				buffer_i += 1;
		} while(!parser_is_R_right_parenthesis(tokens + buffer_i));
	} else {
		parser_create_leaf(
			NodeType_TYPE_PAL_VOID,
			*i, // set the token at the start of the GRP `(`
			&buffer_j,
			stack_operator,
			parser);
	}

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
	*i = buffer_i;
	*j = buffer_j;
	return true;
}

bool if_TYPE_create_operator(
size_t* i,
size_t* j,
MemoryStack* stack_operator,
Parser* parser) {
	assert(i != NULL);
	assert(j != NULL);
	assert(parser != NULL);

	const Token* tokens = (const Token*) parser->lexer->tokens.base;

	if(tokens[*i].type != TokenType_RSPE // TODO RSPE
	&& tokens[*i].type != TokenType_R
	&& tokens[*i].type != TokenType_RSCOPE)
		return false;

	size_t buffer_i = *i;
	size_t buffer_j = *j;
	bool maybe_type_return = false;

	if(if_TYPE_VAR_create_leaf(
		&buffer_i,
		&buffer_j,
		stack_operator,
		parser)
	== true)
		maybe_type_return = true;
	// buffer_i is at `(` if it is a PAL
	if(parser_is_R_left_parenthesis(tokens + buffer_i)) {
		if(!maybe_type_return) {
			parser_create_leaf(
				NodeType_TYPE_PAL_VOID,
				buffer_i,
				&buffer_j,
				stack_operator,
				parser);
		}

		if(if_GRP_RPARES_create_operator(
			&buffer_i,
			&buffer_j,
			stack_operator,
			parser)
		== false) {
			return false;
		}

		Operator* top_operator = memory_stack_top_addr(stack_operator);
		top_operator->count_arity -= 2; // TYPE_PAL = TYPE_VAR + GRP_PARES
		parser_create_operator(
			NodeType_TYPE_PAL,
			2,
			*i, // is left untouch, points to the first R parenthesis or the return type
			&buffer_j,
			stack_operator,
			parser);
		buffer_i += 1;
	}

	*i = buffer_i;
	*j = buffer_j;
	return true;
}

