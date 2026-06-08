#include "parser_node.h"
#include "parser_qualifier.h"
#include "parser_utils.h"

void if_GRP_Q_create_operator(
size_t* j,
size_t i_Q,
MemoryStack* stack_context,
Parser* parser) {
	const Token* const tokens = parser->lexer->tokens.base;
	size_t count_Q = 0;
	size_t buffer_Q = i_Q;
	// consumes all Q
	while(parser_is_Q(tokens + buffer_Q)) {
		parser_create_leaf_raw(
			NodeType_Q,
			buffer_Q,
			j,
			parser);
		buffer_Q += 1;
		count_Q += 1;
	}
	// create a group of Q
	parser_create_operator_raw(
		NodeType_GRP_Q,
		count_Q,
		i_Q,
		j,
		parser);
	Context* top_context = memory_stack_top_addr(stack_context);
	top_context->count_child += 1;
}
