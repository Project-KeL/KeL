#include <stdio.h>
#include "headers/tac_quadruple.h"
#include "debug_tac.h"

#ifndef NDEBUG

static void print_info_quadruple_item(
const char* code,
const Token* token,
const QuadrupleItem* quadruple_item) {
	switch(quadruple_item->type) {
	case QuadrupleItemType_LIT:
		printf(
			"%.*s\t",
			(int)(token->end - token->start),
			code + token->start);
		break;
	case QuadrupleItemType_TEMP:
		printf(
			"t%zu\t",
			quadruple_item->offset_node);
		break;
	default: assert(false);
	}
}

static void print_info_quadruple_entry(
const Parser* parser,
const QuadrupleEntry* quadruple_entry) {
	const char* type;

	switch(quadruple_entry->op.type) {
	case QuadrupleItemType_ADD: type = "ADD"; break;
	case QuadrupleItemType_SUB: type = "SUB"; break;
	case QuadrupleItemType_MUL: type = "MUL"; break;
	case QuadrupleItemType_DIV: type = "DIV"; break;
	default: assert(false);
	}

	const char* code = parser->lexer->source->content;
	const Node* nodes = parser->nodes.base;
	const Token* tokens = parser->lexer->tokens.base;

	const Node* node_src1 = nodes + quadruple_entry->src1.offset_node;
	const Node* node_src2 = nodes + quadruple_entry->src2.offset_node;
	const Node* node_dst = nodes + quadruple_entry->dst.offset_node;

	const Token* token_src1 = tokens + node_src1->offset_token;
	const Token* token_src2 = tokens + node_src2->offset_token;
	const Token* token_dst = tokens + node_dst->offset_token;

	printf(
		"\t%s\t",
		type);
	print_info_quadruple_item(
		code,
		token_src1,
		&quadruple_entry->src1);
	print_info_quadruple_item(
		code,
		token_src2,
		&quadruple_entry->src2);
	print_info_quadruple_item(
		code,
		token_dst,
		&quadruple_entry->dst);
	printf("\n");
}

void debug_print_quadruple_list(const TAC* tac) {
	printf("QUADRUPLES:\n");
	const QuadrupleList* quadruple_list = &tac->quadruple_list;
	QuadrupleEntry* base = quadruple_list->quadruples.area.base;
	size_t count = base == NULL
		? 0
		: (size_t)((const char*) quadruple_list->quadruples.top - (const char*) base) / sizeof(QuadrupleEntry);

	for(
	size_t i = 0;
	i < count;
	i += 1) {
		print_info_quadruple_entry(
			tac->stab.parser,
			base + i);
	}
}

#endif
