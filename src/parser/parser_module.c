#include <assert.h>
#include "parser_allocator.h"
#include "parser_module.h"
#include <stdio.h>

static int module_bind_child_token(
size_t i,
Parser* parser) {
	if(!parser_allocator(parser))
		return -1;

	*((Node*) parser->nodes.top) = (Node) {
		.is_child = true,
		.type = ((Node*) parser->nodes.previous)->subtype,
		.subtype = NodeType_NO,
		.token = (const Token*) parser->lexer->tokens.addr + i,
		.nodes = {[NODE_INDEX_MODULE_TAIL] = NULL}};
	((Node*) parser->nodes.previous)->nodes[NODE_INDEX_MODULE_TAIL] = (Node*) parser->nodes.top;
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

	if(tokens[buffer_i].type != TokenType_L)
		return 0;

	NodeSubtypeModule subtype;

	if(tokens[buffer_i].subtype == TokenSubtype_MODULE_INPUT)
		subtype = NodeSubtypeModule_INPUT;
	else if(tokens[buffer_i].subtype == TokenSubtype_MODULE_OUTPUT)
		subtype = NodeSubtypeModule_OUTPUT;
	else
		return 0;

	buffer_i += 1;
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
			.nodes = {[NODE_INDEX_MODULE_TAIL] = NULL}};
		*node_module_last = (Node*) parser->nodes.top;
		buffer_i += 1;

		while(tokens[buffer_i].type == TokenType_PL) {
			switch(module_bind_child_token(
				buffer_i,
				parser)) {
			case -1: return -1;
			case 1: /* fall through */;
			}

			buffer_i += 1;
		}

		if(tokens[buffer_i].subtype == TokenSubtype_COMMA) {
			buffer_i += 1;
		} else if(tokens[buffer_i].subtype != TokenSubtype_PERIOD
		       && tokens[buffer_i].subtype != TokenSubtype_SEMICOLON) {
			goto RETURN_0;
		}
	} while(tokens[buffer_i].type == TokenType_L);

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

void parser_module_set_tail(
Node* module,
Node* tail) {
	assert(module != NULL);
	assert(tail != NULL);

	module->nodes[NODE_INDEX_MODULE_TAIL] = tail;
}

Node* parser_module_get_tail(const Node* module) {
	assert(module != NULL);

	return module->nodes[NODE_INDEX_MODULE_TAIL];
}
