#include "parser_node.h"
#include "allocator.h"


void parser_create_leaf(
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
	Operator* top_operator = memory_stack_top_addr(stack_operator);
	top_operator->count_arity += 1;
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

	Node* nodes = (Node*) parser->nodes.base;
	nodes[*j] = (Node) {
		.type = type,
		.arity = arity,
		.token = token};
	Operator* top_operator = memory_stack_top_addr(stack_operator);
	top_operator->count_arity += 1;
	*j += 1;
}
