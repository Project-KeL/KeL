#include "parser_scope.h"
#include "allocator.h"
#include "lexer.h"
#include "parser.h"
#include "parser_node.h"
#include "parser_qualifier.h"
#include "parser_utils.h"
#include <stdio.h>

bool if_LSCOPE_create_context(
size_t* i,
MemoryStack* stack_context,
MemoryStack* stack_operator,
Parser* parser) {
	const Token* const tokens = parser->lexer->tokens.base;

	if(!parser_is_LSCOPE_start(tokens + *i))
		return false;

	Context context_scope = (Context) {
		.type = ContextType_SCOPE, // later `then` and `through`
		.watermark = parser_context_get_watermark(stack_operator),
		.count_child = 0,
		.token = *i};
	memory_stack_push(
		(char*) &context_scope,
		stack_context);
	*i += 1;

	return true;
}

bool if_LSCOPE_end_destroy_context(
size_t* i,
size_t* j,
size_t* i_Q,
MemoryStack* stack_context,
MemoryStack* stack_operator,
Parser* parser) {
	const Token* const tokens = parser->lexer->tokens.base;

	if(!parser_is_LSCOPE_end(tokens + *i))
		return false;

	Context* top_context = memory_stack_top_addr(stack_context);
	// is there a scope to be closed?
	if(top_context->type == ContextType_SCOPE_0) {
		return false;
	}

	parser_context_flush(
		j,
		stack_context,
		stack_operator,
		parser);
	// close the scope and add it as an operator
	Context context_scope;
	memory_stack_pop(
		(char*) &context_scope,
		stack_context);
	parser_create_operator(
		NodeType_SCOPE,
		context_scope.count_child,
		context_scope.token,
		j,
		stack_operator,
		parser);
	// if the previous operator is a PAL initialization
	Operator* top_operator = memory_stack_top_addr(stack_operator);

	if(top_operator->type == NodeType_INIT_PAL) {
		Operator pop_operator;
		memory_stack_pop(
			(char*) &pop_operator,
			stack_operator);
		parser_create_operator(
			pop_operator.type,
			1,
			pop_operator.token,
			j,
			stack_operator,
			parser);
		Operator* top_operator = memory_stack_top_addr(stack_operator);
		// a `INIT_VAR` cannot be initialize with a `scope`
		assert(top_operator->type == NodeType_DECL_PAL);
		// pop the `DECL_PAL`
		memory_stack_pop(
			(char*) &pop_operator,
			stack_operator);
		parser_create_operator(
			pop_operator.type,
			pop_operator.count_arity,
			pop_operator.token,
			j,
			stack_operator,
			parser);
		// insert the qualifiers
		if(*i_Q != 0) {
			if_GRP_Q_create_operator(
				j,
				*i_Q,
				stack_context,
				stack_operator,
				parser);
			*i_Q = 0;
		}
	}

	top_context = memory_stack_top_addr(stack_context);
	top_context->count_child += 1;
	*i += 1;
	return true;
}
