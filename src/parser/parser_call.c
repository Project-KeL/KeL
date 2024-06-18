#include <assert.h>
#include <string.h>
#include "parser_allocator.h"
#include "parser_call.h"
#include <stdio.h>

int if_call_create_nodes(
size_t* i,
Parser* parser) {
	return 0;
	const char* code = parser->lexer->source->content;
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;
	size_t buffer_i = *i;
	const MemoryChainLink* declaration_link;
	const Node* declaration_node = parser_allocator_start_declaration(
		parser,
		&declaration_link);

	if(declaration_node == NULL)
		return 0;

	while(parser_allocator_continue_declaration(
		parser,
		declaration_node)
	== true) {
		if(declaration_node->type == NodeType_NO)
			break;

		if(strncmp(
			code + declaration_node->token->L_start,
			code + tokens[buffer_i].L_start,
			declaration_node->token->L_end - declaration_node->token->L_start)
		== 0)
			goto FOUND;

		if(parser_allocator_next(
			parser,
			&declaration_link,
			&declaration_node)
		== false)
			break;
	}

	return 0;
FOUND:
	printf("FOUND!\n");
	*i = buffer_i + 1;
	return 1;
}
