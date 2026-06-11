#include "allocator.h"
#include "parser_node.h"
#include "parser_qualifier.h"
#include "parser_utils.h"

bool if_GRP_Q_create_operator(
size_t* i,
size_t* j,
MemoryStack* stack_context,
MemoryStack* stack_operator,
Parser* parser) {
	const Token* const tokens = parser->lexer->tokens.base;
	size_t buffer_i = *i;
	Operator operator_GRP_Q = (Operator) {
		.type = NodeType_GRP_Q,
		.precedence = 0,
		.count_arity = 0,
		.token = buffer_i};

	MemoryStackState stack_operator_state;
	initialize_memory_stack_state(&stack_operator_state);
	memory_stack_state_save(
		stack_operator,
		&stack_operator_state);

	memory_stack_push(
		(char*) &operator_GRP_Q,
		stack_operator);
	// consumes all Q
	if(parser_is_Q(tokens + buffer_i)) {
		do {
			parser_create_leaf(
				NodeType_Q,
				buffer_i,
				j,
				stack_operator,
				parser);
			buffer_i += 1;
		} while(parser_is_Q(tokens + buffer_i));
	} else {
		memory_stack_state_restore(
			stack_operator,
			&stack_operator_state);
		return false;
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
	*i = buffer_i;
	return true;
}
