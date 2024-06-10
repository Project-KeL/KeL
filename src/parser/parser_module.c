#include <assert.h>
#include "parser_allocator.h"
#include "parser_module.h"

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
			.token = tokens + buffer_i};
		buffer_i += 1;

		if(tokens[buffer_i].subtype != TokenSubtype_COMMA)
			break;

		buffer_i += 1;
	} while(tokens[buffer_i].type == TokenType_L);

	*i = buffer_i;
	return 1;
}
