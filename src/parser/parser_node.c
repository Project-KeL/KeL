#include "parser_node.h"
#include "allocator.h"
#include <stdio.h>

void parser_create_leaf_raw(
NodeType type,
size_t token,
size_t* j,
Parser* parser) {
	assert(j != NULL);
	assert(parser != NULL);
	assert(*j < parser->nodes.count);

	Node* nodes = parser->nodes.base;
	nodes[*j] = (Node) {
		.type = type,
		.arity = 0,
		.offset_token = token};
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
		parser);
	Operator* top_operator = memory_stack_top_addr(stack_operator);
	top_operator->count_arity += 1;
}

void parser_create_operator_raw(
NodeType type,
uint32_t arity,
size_t token,
size_t* j,
Parser* parser) {
	assert(j != NULL);
	assert(parser != NULL);
	assert(*j < parser->nodes.count);

	Node* nodes = parser->nodes.base;
	nodes[*j] = (Node) {
		.type = type,
		.arity = arity,
		.offset_token = token};
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
		parser);

	if(memory_stack_is_empty(stack_operator))
		return;

	Operator* top_operator = memory_stack_top_addr(stack_operator);
	top_operator->count_arity += 1;
}

size_t parser_context_get_watermark(const MemoryStack* stack_operator) {
	const MemoryArea* const area = &stack_operator->area;
	return ((char*) stack_operator->top - (char*) area->base) / area->size_type;
}	

void parser_context_flush(
size_t* j,
MemoryStack* stack_context,
MemoryStack* stack_operator,
Parser* parser) {
	assert(j != NULL);
	assert(stack_context != NULL);
	assert(stack_operator != NULL);
	assert(parser != NULL);

	Context* top_context = memory_stack_top_addr(stack_context);
	size_t watermark_over = parser_context_get_watermark(stack_operator);

	while(watermark_over > top_context->watermark) {
		Operator pop_operator;
		memory_stack_pop(
			(char*) &pop_operator,
			stack_operator);
		parser_create_operator_raw(
			pop_operator.type,
			pop_operator.count_arity,
			pop_operator.offset_token,
			j,
			parser);
		top_context->count_child += 1;
		watermark_over -= 1;
	}
}
