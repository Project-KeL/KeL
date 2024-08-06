#include <assert.h>
#include "parser_allocator.h"
#include "parser_scope.h"
#include "parser_utils.h"

static Node* get_scope_from_period(Parser* parser) {
	Node* node = (Node*) parser->nodes.top;
	MemoryChainLink* restrict link = parser->nodes.last;
	size_t count_scope_nest = 1;
	uint64_t count = 0;

	do {
		// get the last node in the previous memory area
		parser_allocator_node_previous(
			&node,
			&link);

		if(!node->is_child) {
			switch(node->type) {
			case NodeType_SCOPE_END: count_scope_nest += 1; break;
			case NodeType_SCOPE_START: count_scope_nest -= 1; break;
			}
		}

		count += 1;
	} while(count_scope_nest != 0
	     || node->type != NodeType_SCOPE_START);

	node->value = count;
	return node;
}

int if_scope_create_node(
size_t i,
Parser* parser) {
	assert(parser != NULL);

	const Token* token = (Token*) parser->lexer->tokens.addr + i;

	if(!parser_is_scope_L(token))
		return 0;

	if(!parser_allocator(parser))
		return -1;

	*((Node*) parser->nodes.top) = (Node) {
		.is_child = false,
		.type = NodeType_SCOPE_START,
		.subtype = NodeSubtypeScope_NO,
		.value = 0,
		.child1 = NULL,
		.child2 = NULL};
	return 1;
}

int if_period_create_node(
size_t i,
Parser* parser) {
	assert(parser != NULL);

	const Token* token = (Token*) parser->lexer->tokens.addr + i;

	if(token->subtype != TokenSubtype_PERIOD)
		return 0;

	if(!parser_allocator(parser))
		return -1;

	Node* scope = get_scope_from_period(parser);
	*((Node*) parser->nodes.top) = (Node) {
		.is_child = false,
		.type = NodeType_SCOPE_END,
		.subtype = scope->subtype};
	scope->child1 = (Node*) parser->nodes.top;
	return 1;
}
