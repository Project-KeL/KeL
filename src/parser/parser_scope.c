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
		.ScopeStart = {
			.scope_end = NULL,
			.PAL = NULL}};
#ifndef NDEBUG
	parser_is_valid_scope_start((const Node*) parser->nodes.top);
#endif
	return 1;
}

bool parser_is_scope_start(const Node* node) {
	return !node->is_child
	    && node->type == NodeType_SCOPE_START
        && (node->subtype == NodeSubtypeScopeStart_NO
	     || node->subtype == NodeSubtypeScopeStart_THEN
	     || node->subtype == NodeSubtypeScopeStart_THEN_NOT
	     || node->subtype == NodeSubtypeScopeStart_THROUGH
	     || node->subtype == NodeSubtypeScopeStart_THROUGH_NOT
	     || node->subtype == NodeSubtypeScopeStart_TEST);
}

bool parser_is_valid_scope_start(const Node* node) {
	assert(parser_is_scope_start(node));
	
	return true;
}

void parser_scope_start_set_scope_end(
Node* node,
Node* scope_end) {
#ifndef NDEBUG
	parser_is_valid_scope_start(node);
	parser_is_valid_scope_end(scope_end);
#endif
	node->ScopeStart.scope_end = scope_end;
}

void parser_scope_start_set_PAL(
Node* node,
Node* PAL) {
#ifndef NDEBUG
	parser_is_valid_scope_start(node);
#endif
	node->ScopeStart.PAL = PAL;
}

const Node* parser_scope_start_get_scope_end(const Node* node) {
#ifndef NDEBUG
	parser_is_valid_scope_start(node);
#endif
	return node->ScopeStart.scope_end;
}

const Node* parser_scope_start_get_PAL(const Node* node) {
#ifndef NDEBUG
	parser_is_valid_scope_start(node);
#endif
	return node->ScopeStart.PAL;
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
#ifndef NDEBUG
	parser_is_valid_scope_end((const Node*) parser->nodes.top);
#endif
	return 1;
}

bool parser_is_scope_end(const Node* node) {
	return !node->is_child
	    && node->type == NodeType_SCOPE_END;
}

bool parser_is_valid_scope_end(const Node* node) {
	assert(parser_is_scope_end(node));

	return true;
}

void parser_scope_end_set_scope_start(
Node* node,
Node* scope_start) {
#ifndef NDEBUG
	parser_is_valid_scope_end(node);
	parser_is_valid_scope_start(scope_start);
#endif
	node->ScopeEnd.scope_start = scope_start;
}

const Node* parser_scope_end_get_scope_start(const Node* node) {
#ifndef NDEBUG
	parser_is_valid_scope_end(node);
#endif
	return node->ScopeEnd.scope_start;
}
