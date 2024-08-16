#include <assert.h>
#include "parser_allocator.h"
#include "parser_module.h"

static int module_bind_child_module(
size_t i,
Parser* parser) {
	const Token* token = (const Token*) parser->lexer->tokens.addr + i;

	if(token->type != TokenType_PL)
		return 0;

	if(!parser_allocator(parser))
		return -1;

	*((Node*) parser->nodes.top) = (Node) {
		.is_child = true,
		.type = NodeType_MODULE,
		.subtype = ((Node*) parser->nodes.previous)->subtype,
		.token = token,
		.Module = {.next = NULL}};
	((Node*) parser->nodes.previous)->Module.next = (Node*) parser->nodes.top;
	return 1;
}

int if_module_create_nodes(
size_t* i,
Node** node_module_last,
Parser* parser) {
	assert(i != NULL);
	assert(node_module_last != NULL);
	assert(parser != NULL);

	size_t buffer_i = *i;
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;

	if(tokens[buffer_i].subtype != TokenSubtype_MODULE_INPUT
	&& tokens[buffer_i].subtype != TokenSubtype_MODULE_OUTPUT)
		return 0;

	NodeSubtypeModule subtype;

	if(tokens[buffer_i].subtype == TokenSubtype_MODULE_INPUT)
		subtype = NodeSubtypeModule_INPUT;
	else
		subtype = NodeSubtypeModule_OUTPUT;

	buffer_i += 1;

	if(tokens[buffer_i].type != TokenType_L)
		return 0;

	MemoryChainState memChain_state;
	initialize_memory_chain_state(&memChain_state);
	memory_chain_state_save(
		&parser->nodes,
		&memChain_state);

	do {
		if(!parser_allocator(parser))
			return -1;
	
		*((Node*) parser->nodes.top) = (Node) {
			.is_child = false,
			.type = NodeType_MODULE,
			.subtype = subtype,
			.token = tokens + buffer_i,
			.Module = {.next = NULL}};
		buffer_i += 1;
		*node_module_last = (Node*) parser->nodes.top;
		int error;

		while((error = module_bind_child_module(
			buffer_i,
			parser))
		== 1) {
			switch(error) {
			case -1: return -1;
			case 0: goto RETURN_0;
			case 1: /* fall through */;
			}

			buffer_i += 1;
		}

		if(tokens[buffer_i].subtype != TokenSubtype_COMMA)
			break;

		buffer_i += 1;
	} while(tokens[buffer_i].type == TokenType_L);
#ifndef NDEBUG
	parser_is_valid_module(*node_module_last);
#endif
	*i = buffer_i;
	return 1;
RETURN_0:
	memory_chain_state_restore(
		&parser->nodes,
		&memChain_state);
	return 0;
}

bool parser_is_module(const Node* node) {
	return !node->is_child
	    && node->type == NodeType_MODULE
	    && (node->subtype == NodeSubtypeModule_INPUT
	     || node->subtype == NodeSubtypeModule_OUTPUT);
}

bool parser_is_valid_module(const Node* node) {
	assert(parser_is_module(node));

	return true;
}

void parser_module_set_next(
Node* node,
Node* next) {
#ifndef NDEBUG
	parser_is_valid_module(node);
	parser_is_valid_module(next);
#endif
	node->Module.next = next;
}

const Node* parser_module_get_next(const Node* node) {
#ifndef NDEBUG
	parser_is_valid_module(node);
#endif
	return node->Module.next;
}
