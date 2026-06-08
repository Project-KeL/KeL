#include "allocator.h"
#include "parser_node.h"
#include "parser_qualifier.h"
#include "parser_utils.h"

void if_GRP_Q_create_operator(
size_t* j,
size_t i_Q,
MemoryStack* stack_context,
MemoryStack* stack_operator,
Parser* parser) {
	const Token* const tokens = parser->lexer->tokens.base;

	Operator operator_GRP_Q = (Operator) {
		.type = NodeType_GRP_Q,
		.precedence = 0,
		.count_arity = 0,
		.token = i_Q};
	memory_stack_push(
		(char*) &operator_GRP_Q,
		stack_operator);
	size_t buffer_Q = i_Q;
	// consumes all Q
	while(parser_is_Q(tokens + buffer_Q)) {
		parser_create_leaf(
			NodeType_Q,
			buffer_Q,
			j,
			stack_operator,
			parser);
		buffer_Q += 1;
	}
	// create a group of Q
	Operator pop_operator_GRP_Q;
	memory_stack_pop(
		(char*) &pop_operator_GRP_Q,
		stack_operator);
	parser_create_operator_raw(
		pop_operator_GRP_Q.type,
		pop_operator_GRP_Q.count_arity,
		pop_operator_GRP_Q.token,
		j,
		parser);
	Context* top_context = memory_stack_top_addr(stack_context);
	top_context->count_child += 1;
}
