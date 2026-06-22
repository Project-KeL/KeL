#include "parser.h"
#ifndef NDEBUG
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include "allocator.h"
#include "debug_parser.h"

#define RED "\x1b[31m"
#define RESET "\x1b[0m"

static void print_info_node(
const Parser* parser,
const Node* node) {
	const char* type;

	switch(node->type) {
// Q
	case NodeType_Q: type ="QUAL"; break;
// LIT
	case NodeType_LIT_NUM: type = "LIT NUM"; break;
	case NodeType_LIT_CHAR: type = "LIT CHAR"; break;
	case NodeType_LIT_STR: type = "LIT STR"; break;
// GRP
	case NodeType_GRP_IMOD: type = "GRP IMOD"; break;
	case NodeType_GRP_OMOD: type = "GRP OMOD"; break;
	case NodeType_GRP_Q: type = "GRP QUAL"; break;
	case NodeType_GRP_L_PARES: type = "GRP L PARES"; break;
	case NodeType_GRP_R_PARES: type = "GRP R PARES"; break;
	case NodeType_GRP_CALL_ARGS: type = "GRP CALL ARG NONE"; break;
// MOD
	case NodeType_IMOD: type = "IMOD"; break;
	case NodeType_OMOD: type = "OMOD"; break;
// SCP
	case NodeType_SCOPE: type = "SCOPE"; break;
	case NodeType_SCOPE_IF: type = "IF"; break;
	case NodeType_SCOPE_THROUGH: type = "THROUGH"; break;
	case NodeType_SCOPE_ELSE_IF: type = "ELSE_IF"; break;
	case NodeType_SCOPE_ELSE: type = "ELSE"; break;
	case NodeType_SCOPE_ELSE_THROUGH: type = "ELSE THROUGH"; break;
	case NodeType_SCOPE_END: type = "SCOPE END"; break;
// EXP
	case NodeType_EXP: type = "EXP"; break;
// Keys actions
	case NodeType_DECL_VAR: type = "DECL VAR"; break;
	case NodeType_DECL_PAL: type = "DECL PAL"; break;
	case NodeType_INIT_VAR: type = "INIT VAR"; break;
	case NodeType_INIT_PAL: type = "INIT PAL"; break;
	case NodeType_TYPE_VAR: type = "TYPE VAR"; break;
	case NodeType_TYPE_PAL: type = "TYPE PAL"; break;
	case NodeType_TYPE_PAL_VOID: type = "TYPE PAL VOID"; break;
	case NodeType_ID: type = "ID"; break;
	case NodeType_KEY: type = "KEY"; break;
	case NodeType_PARAM: type = "PARAM"; break;
	case NodeType_CALLEE: type = "CALEE"; break;
// OP (parenthesis should not be encountered: RPN)
	case NodeType_OP_ASSIGN: type = "ASSIGN"; break;
	case NodeType_OP_ADD: type = "ADD"; break;
	case NodeType_OP_SUB: type = "SUB"; break;
	case NodeType_OP_MUL: type = "MUL"; break;
	case NodeType_OP_DIV: type = "DIV"; break;
// error
	case NodeType_OP_LPARENTHESIS:
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
		(int)(tokens[node->offset_token].end - tokens[node->offset_token].start),
		lexer->source->content + tokens[node->offset_token].start);
}

void debug_print_nodes(const Parser* parser) {
	printf("NODES:\n");

	for(size_t i = 1;
	i < parser->nodes.count - 1;
	i += 1) {
		printf(
			"%zu\t",
			i);
		print_info_node(
			parser,
			(Node*) parser->nodes.base + i);
	}

	printf(
		"\nNumber of nodes: %zu.\n",
		parser->nodes.count - 2); // null nodes at start and end
}

void debug_print_tree(const Parser* parser) {
	printf("TREE:\n");
	const Node* nodes = parser->nodes.base;
	const size_t count = parser->nodes.count;
	size_t* start_subtree = calloc(
		count,
		sizeof(size_t));
	size_t* stack_index = malloc(count * sizeof(size_t));
	size_t* stack_depth = malloc(count * sizeof(size_t));

	if(start_subtree == NULL
	|| stack_depth == NULL
	|| stack_index == NULL)
		goto END;

	{
		size_t top = 0;

		for(size_t k = 1;
		k < count - 1;
		k += 1) {
			size_t start = k;
			for(uint32_t c = 0;
			c < nodes[k].arity;
			c += 1) {
				top -= 1;
				start = stack_index[top];
			}

			start_subtree[k] = start;
			stack_index[top] = start;
			top += 1;
		}
	}

	size_t top = 0;

	for(size_t end = count - 2
	;
	;) {
		stack_index[top] = end;
		stack_depth[top] = 0;
		top += 1;
		size_t start = start_subtree[end];

		if(start <= 1)
			break;

		end = start - 1;
	}

	while(top != 0) {
		top -= 1;
		const size_t i = stack_index[top];
		const size_t depth = stack_depth[top];
		printf("\t");

		for(size_t d = 0;
		d < depth;
		d += 1) {
			printf("  ");
		}

		print_info_node(
			parser,
			nodes + i);
		size_t end = i - 1;

		for(uint32_t c = 0;
		c < nodes[i].arity;
		c += 1) {
			stack_index[top] = end;
			stack_depth[top] = depth + 1;
			top += 1;
			const size_t start = start_subtree[end];

			if(start <= 1)
				break;

			end = start - 1;
		}
	}
END:
	free(stack_depth);
	free(stack_index);
	free(start_subtree);
}

// make a better tree with:
// ─
// ├
// └

#endif
