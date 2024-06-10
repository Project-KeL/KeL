#include <assert.h>
#include "parser_allocator.h"
#include "parser_scope.h"
#include "parser_utils.h"

static Node* parser_get_scope_from_period(Parser* parser) {
	Node* node = (Node*) parser->nodes.top;
	MemoryChainLink* restrict link = parser->nodes.last;
	size_t count_scope_nest = 1;
	uint64_t count = 0;

	do {
		// get the last node in the previous memory area
		if(node == link->memArea.addr) {
			link = link->previous;
			node = (Node*) link->memArea.addr + link->memArea.count - 1;
		} else
			node -= 1;

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

bool if_scope_create_node(
size_t i,
Parser* parser) {
	assert(parser != NULL);

	const Token* token = (Token*) parser->lexer->tokens.addr + i;

	if(!parser_is_scope_L(token))
		return false;

	if(!parser_allocator_node(parser))
		return false;

	*((Node*) parser->nodes.top) = (Node) {
		.is_child = false,
		.type = NodeType_SCOPE_START,
		.subtype = NodeSubtypeScope_NO,
		.value = 0,
		.child = NULL};
	return true;
}

int if_period_create_node(
size_t i,
Parser* parser) {
	assert(parser != NULL);

	const Token* token = (Token*) parser->lexer->tokens.addr + i;

	if(token->subtype != TokenSubtype_PERIOD)
		return 0;

	if(!parser_allocator_node(parser))
		return -1;

	Node* scope = parser_get_scope_from_period(parser);
	*((Node*) parser->nodes.top) = (Node) {
		.is_child = false,
		.type = NodeType_SCOPE_END,
		.subtype = scope->subtype};
	scope->child = (Node*) parser->nodes.top;
	return 1;
}
