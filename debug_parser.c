#include "parser.h"
#ifndef NDEBUG
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include "debug_parser.h"
#include "parser_allocator.h"

static void print_info_node(
const Parser* parser,
const Node* node) {
	const char* type;

	switch(node->type) {
	case NodeType_Q: type ="QUAL"; break;
	case NodeType_LIT_NUM: type = "LIT NUM"; break;
	case NodeType_LIT_CHAR: type = "LIT CHAR"; break;
	case NodeType_LIT_STR: type = "LIT STR"; break;
	case NodeType_GRP_Q: type = "GRP QUAL"; break;
	case NodeType_GRP_L_PARES: type = "GRP L PARES"; break;
	case NodeType_GRP_R_PARES: type = "GRP R PARES"; break;
	case NodeType_GRP_BRACS: type = "GRP BRACS"; break;
	case NodeType_SCOPE: type = "SCOPE"; break;
	case NodeType_SCOPE_IF: type = "IF"; break;
	case NodeType_SCOPE_THROUGH: type = "THROUGH"; break;
	case NodeType_SCOPE_ELSE_IF: type = "ELSE_IF"; break;
	case NodeType_SCOPE_ELSE: type = "ELSE"; break;
	case NodeType_SCOPE_ELSE_THROUGH: type = "ELSE THROUGH"; break;
	case NodeType_DECL_VAR: type = "DECL VAR"; break;
	case NodeType_DECL_PAL: type = "DECL PAL"; break;
	case NodeType_INIT_VAR: type = "INIT VAR"; break;
	case NodeType_INIT_PAL: type = "INIT PAL"; break;
	case NodeType_TYPE_VAR: type = "TYPE VAR"; break;
	case NodeType_TYPE_PAL: type = "TYPE PAL"; break;
	case NodeType_TYPE_PAL_VOID: type = "TYPE PAL VOID"; break;
	case NodeType_ID: type = "ID"; break;
	case NodeType_L: type = "L"; break;
	case NodeType_PARAM: type = "PARAM"; break;
	case NodeType_CALL: type = "CALL"; break;
	default: assert(false);
	}

	printf("%s", type);

	if(node->arity > 0) {
		printf("/%" PRIu32, node->arity);
	}
	
	const Lexer* lexer = parser->lexer;
	const Token* tokens = lexer->tokens.base;

	printf(
		" \"%.*s\"\n",
		(int)(tokens[node->token].end - tokens[node->token].start),
		lexer->source->content + tokens[node->token].start);
}

void debug_print_nodes(const Parser* parser) {
	printf("NODES:\n");

	for(size_t i = 1;
	i < parser->nodes.count - 1;
	i += 1) {
		printf("\t");
		print_info_node(
			parser,
			(Node*) parser->nodes.base + i);
	}

	printf(
		"\nNumber of nodes: %zu.\n",
		parser->nodes.count - 2); // null nodes at start and end
}

#endif
