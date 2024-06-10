#include <assert.h>
#include "parser_allocator.h"
#include "parser_module.h"
#include <stdio.h>

static int module_bind_child_module(
size_t i,
Parser* parser) {
	const Token* token = (const Token*) parser->lexer->tokens.addr + i;

	if(token->type != TokenType_PL)
		return 0;

	if(!parser_allocator_node(parser))
		return -1;

	*((Node*) parser->nodes.top) = (Node) {
		.is_child = true,
		.type = NodeType_MODULE,
		.subtype = NodeSubtypeModule_INPUT,
		.token = token};
	((Node*) parser->nodes.previous)->child = (Node*) parser->nodes.top;
	return 1;
}

int if_module_create_nodes(
size_t* i,
Parser* parser) {
	assert(i != NULL);
	assert(parser != NULL);

	size_t buffer_i = *i;
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;

	if(tokens[buffer_i].subtype != TokenSubtype_MODULE_INPUT)
		return 0;

	buffer_i += 1;

	if(tokens[buffer_i].type != TokenType_L)
		return 0;

	do {
		if(!parser_allocator_node(parser))
			return -1;

		*((Node*) parser->nodes.top) = (Node) {
			.is_child = false,
			.type = NodeType_MODULE,
			.subtype = NodeSubtypeModule_INPUT,
			.token = tokens + buffer_i,
			.child = NULL};
		buffer_i += 1;
		int error;

		while((error = module_bind_child_module(
			buffer_i,
			parser))
		== 1) {
			if(error == -1)
				return -1;

			buffer_i += 1;
		}

		if(tokens[buffer_i].subtype != TokenSubtype_COMMA)
			break;

		buffer_i += 1;
	} while(tokens[buffer_i].type == TokenType_L);

	*i = buffer_i;
	return 1;
}
