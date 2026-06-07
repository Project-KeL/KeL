#include "parser_node.h"
#include "allocator.h"

void parser_create_leaf_raw(
NodeType type,
size_t token,
size_t* j,
MemoryStack* stack_operator,
Parser* parser) {
	assert(j != NULL);
	assert(stack_operator != NULL);
	assert(parser != NULL);

	Node* nodes = parser->nodes.base;
	nodes[*j] = (Node) {
		.type = type,
		.arity = 0,
		.token = token};
	*j += 1;
}

void parser_create_leaf(
NodeType type,
size_t token,
size_t* j,
MemoryStack* stack_operator,
Parser* parser) {
	assert(j != NULL);
	assert(stack_operator != NULL);
	assert(parser != NULL);

	parser_create_leaf_raw(
		type,
		token,
		j,
		stack_operator,
		parser);
	Operator* top_operator = memory_stack_top_addr(stack_operator);
	top_operator->count_arity += 1;
}

void parser_create_operator_raw(
NodeType type,
uint32_t arity,
size_t token,
size_t* j,
MemoryStack* stack_operator,
Parser* parser) {
	assert(j != NULL);
	assert(stack_operator != NULL);
	assert(parser != NULL);

	Node* nodes = parser->nodes.base;
	nodes[*j] = (Node) {
		.type = type,
		.arity = arity,
		.token = token};
	*j += 1;
}

void parser_create_operator(
NodeType type,
uint32_t arity,
size_t token,
size_t* j,
MemoryStack* stack_operator,
Parser* parser) {
	assert(j != NULL);
	assert(stack_operator != NULL);
	assert(parser != NULL);

	parser_create_operator_raw(
		type,
		arity,
		token,
		j,
		stack_operator,
		parser);
	Operator* top_operator = memory_stack_top_addr(stack_operator);
	top_operator->count_arity += 1;
}

size_t parser_context_get_watermark(const MemoryStack* stack_operator) {
	const MemoryArea* const memArea = &stack_operator->memArea;
	return ((char*) stack_operator->top - (char*) memArea->base) / memArea->size_type;
}	

void parser_context_flush(
size_t* j,
MemoryStack* stack_context,
MemoryStack* stack_operator,
Parser* parser) {
	Context* top_context = memory_stack_top_addr(stack_context);
	const MemoryArea* memArea = &stack_operator->memArea;
	size_t watermark_over = ((char*) stack_operator->top - (char*) memArea->base) / memArea->size_type;

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
}}
