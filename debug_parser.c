#ifndef NDEBUG
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include "debug_parser.h"
#include "parser_allocator.h"
#include "parser.h"
/*
void debug_print_nodes(const TAC* tac) {
	// const char* code = parser->lexer->source->content;

	const MemoryChainLink* link;
	const Node* node = parser_allocator_start_node(
		parser,
		&link);
	printf("NODES:\n");

	if(node == NULL)
		return;
/*
	do {
		printf("\t");

		if(node->type == NodeType_MODULE) {
			if(node->subtype == NodeSubtypeModule_INPUT)
				printf("IMOD ");
			else if(node->subtype == NodeSubtypeModule_OUTPUT)
				printf("OMOD ");

			printf(
				"<%.*s>\n",
				(int) (node->token->L_end - node->token->L_start),
				code + node->token->L_start);
			print_info_node_full_tail(
				code,
				parser,
				&link,
				&node,
				print_info_submodule);
		} else if(node->type == NodeType_SCOPE_START) {
			printf(
				"SCOPE START (%td NODES) ID: %p\n",
				node->value - 1,
				node);
		} else if(node->type == NodeType_INTRODUCTION) {
			print_info_node_key_introduction(
				code,
				node);
			parser_allocator_next(
				parser,
				&link,
				&node);
			printf("\t\t[TYPE]\n");
			print_info_node_full_tail(
				code,
				parser,
				&link,
				&node,
				print_info_node_type);
		} else if(node->type == NodeType_CALL) {
			print_info_node_key_call(
				code,
				node);
			parser_allocator_next(
				parser,
				&link,
				&node);
			printf("\t\t");
			print_info_node_key_call_return_type(
				code,
				node);

			do {
				parser_allocator_next(
					parser,
					&link,
					&node);	
				printf("\t\t");
				print_info_node_argument(
					code,
					node);
			} while(parser_node_get_tail(node) != NULL);
		} else if(node->type == NodeType_SCOPE_END) {
			printf("SCOPE END\n");
		} else if(node->type == NodeType_LITERAL) {
			print_info_token(
				code,
				node->token);
		} else {
			printf(
				"%" PRIu64 ", %" PRIu64 "\n",
				node->type,
				node->subtype);
		}
	} while(parser_allocator_next(
		parser,
		&link,
		&node)
	== true);
*/
	// (number of memory area - 1) * size of a chunk
	// + what remains in the last memory area
/*
	printf(
		"\nNumber of nodes: %zu.\n",
		parser->nodes.count * parser->nodes.first->memArea.count);
}
*/
#endif
