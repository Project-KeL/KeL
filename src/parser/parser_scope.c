#include <assert.h>
#include "parser_allocator.h"
#include "parser_scope.h"
#include "parser_utils.h"

static Node* get_scope_start_from_scope_end(Parser* parser) {
	Node* node = (Node*) parser->nodes.top;
	MemoryChainLink* restrict link = parser->nodes.last;
	size_t count_scope_nest = 1;
	uint64_t count = 0;

	do {
		// get the last node in the previous memory area
		parser_allocator_node_previous(
			&link,
			&node);

		if(!node->is_child) {
			switch(node->type) {
			case NodeType_SCOPE_START: count_scope_nest -= 1; break;
			case NodeType_SCOPE_END: count_scope_nest += 1; break;
			}
		}

		count += 1;
	} while(count_scope_nest != 0
	     || node->type != NodeType_SCOPE_START);

	node->value = count;
	return node;
}

int if_scope_start_create_node(
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
		.subtype = NodeSubtypeScopeStart_NO,
		.value = 0,
		.nodes = {
			[NODE_INDEX_SCOPE_START_SCOPE_END] = NULL,
			[NODE_INDEX_SCOPE_START_PAL] = NULL}};
	return 1;
}

bool parser_is_scope_start(const Node* node) {
	assert(node != NULL);

	return !node->is_child
	    && node->type == NodeType_SCOPE_START
        && (node->subtype == NodeSubtypeScopeStart_NO
	     || node->subtype == NodeSubtypeScopeStart_THEN
	     || node->subtype == NodeSubtypeScopeStart_THEN_NOT
	     || node->subtype == NodeSubtypeScopeStart_THROUGH
	     || node->subtype == NodeSubtypeScopeStart_THROUGH_NOT
	     || node->subtype == NodeSubtypeScopeStart_TEST);
}

void parser_scope_start_set_scope_end(
Node* scope_start,
Node* scope_end) {
	assert(scope_start != NULL);
	assert(scope_end != NULL);

	scope_start->nodes[NODE_INDEX_SCOPE_START_SCOPE_END] = scope_end;
}

void parser_scope_start_set_PAL(
Node* scope_start,
Node* PAL) {
	assert(scope_start != NULL);
	assert(PAL != NULL);

	scope_start->nodes[NODE_INDEX_SCOPE_START_PAL] = PAL;
}

const Node* parser_scope_start_get_scope_end(const Node* scope_start) {
	assert(scope_start  != NULL);

	return scope_start->nodes[NODE_INDEX_SCOPE_START_SCOPE_END];
}

const Node* parser_scope_start_get_PAL(const Node* scope_start) {
	assert(scope_start != NULL);

	return scope_start->nodes[NODE_INDEX_SCOPE_START_PAL];
}

int if_scope_end_create_node(
size_t i,
Parser* parser) {
	assert(parser != NULL);

	const Token* token = (Token*) parser->lexer->tokens.addr + i;

	if(token->subtype != TokenSubtype_PERIOD)
		return 0;

	if(!parser_allocator(parser))
		return -1;

	Node* scope = get_scope_start_from_scope_end(parser);
	*((Node*) parser->nodes.top) = (Node) {
		.is_child = false,
		.type = NodeType_SCOPE_END,
		.subtype = scope->subtype};
	parser_scope_start_set_scope_end(
		scope,
		(Node*) parser->nodes.top);
	return 1;
}

bool parser_is_scope_end(const Node* node) {
	assert(node != NULL);

	return !node->is_child
	    && node->type == NodeType_SCOPE_END;
}

void parser_scope_end_set_scope_start(
Node* scope_end,
Node* scope_start) {
	assert(scope_end != NULL);
	assert(scope_start != NULL);

	scope_end->nodes[NODE_INDEX_SCOPE_END_SCOPE_START] = scope_start;
}

Node* parser_scope_end_get_scope_start(const Node* scope_end) {
	assert(scope_end != NULL);

	return scope_end->nodes[NODE_INDEX_SCOPE_END_SCOPE_START]; 
}
