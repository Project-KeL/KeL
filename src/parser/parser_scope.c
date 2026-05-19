#include "parser_scope.h"
#include "allocator.h"
#include "lexer.h"
#include "parser.h"
#include "parser_node.h"
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

	const MemoryArea* const memArea = &stack_operator->memArea;
	const size_t watermark = ((char*) stack_operator->top - (char*) memArea->base) / memArea->size_type;
	Context context = {
		.type = ContextType_SCOPE, // later `then`
		.watermark = watermark,
		.count_child = 0,
		.token = *i};
	memory_stack_push(
		(char*) &context,
		stack_context);
	*i += 1;
	return true;
}

bool if_LSCOPE_end_destroy_context(
size_t* i,
size_t* j,
MemoryStack* stack_context,
MemoryStack* stack_operator,
Parser* parser) {
	const Token* const tokens = parser->lexer->tokens.base;

	if(!parser_is_LSCOPE_end(tokens + *i))
		return false;

	Context* top_context = memory_stack_top_addr(stack_context);
	// is there a scope to be closed?
	if(top_context->type == ContextType_SCOPE_0)
		return false;

	const MemoryArea* const memArea = &stack_operator->memArea;
	size_t watermark_over = ((char*) stack_operator->top - (char*) memArea->base) / memArea->size_type;
	// pop operators above the watermark
	while(watermark_over > top_context->watermark) {
		Operator pop_operator;
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
		top_context->count_child += 1;
		watermark_over -= 1;
	}
	// close the scope and add it as an operator
	Context context;
	memory_stack_pop(
		(char*) &context,
		stack_context);
	parser_create_operator(
		NodeType_SCOPE,
		context.count_child,
		context.token,
		j,
		stack_operator,
		parser);
	// this previous child is a child of the outer scope
	top_context = memory_stack_top_addr(stack_context);
	top_context->count_child += 1;
	*i += 1;
	return true;
}
