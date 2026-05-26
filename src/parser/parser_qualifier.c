#include "parser_qualifier.h"
#include "parser_utils.h"

void if_GRP_Q_create_operator(
size_t* j,
size_t i_Q,
MemoryStack* stack_context,
Parser* parser) {
	const Token* const tokens = parser->lexer->tokens.base;
	Node* nodes = parser->nodes.base;
	size_t count_Q = 0;
	size_t buffer_Q = i_Q;

	while(parser_is_Q(tokens + buffer_Q)) {
		nodes[*j] = (Node) {
			.type = NodeType_Q,
			.arity = 0,
			.token = buffer_Q};
		*j += 1;
		buffer_Q += 1;
		count_Q += 1;
	}

	nodes[*j] = (Node) {
		.type = NodeType_GRP_Q,
		.arity = count_Q,
		.token = i_Q};
	*j += 1;

	Context* top_context = memory_stack_top_addr(stack_context);
	top_context->count_child += 1;
}
