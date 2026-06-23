#include <stdio.h>
#include "tac.h"
#include "tac_quadruple.h"

#ifndef NDEBUG

static void print_info_quaditem(
const char* code,
const Token* token,
const QuadItem* quaditem) {
	printf("\t\t");

	switch(quaditem->type) {
	case QuadItemType_NO: break;
	case QuadItemType_KEY:
	case QuadItemType_LIT:
	case QuadItemType_PAL:
		printf(
			"%.*s",
			(int)(token->end - token->start),
			code + token->start);
		break;
	case QuadItemType_TEMP:
		printf(
			"t%zu",
			quaditem->offset_node);
		break;
	case QuadItemType_CALL:
		printf(
			"%.*s",
			(int)(token->end - token->start),
			code + token->start);
		break;
	case QuadItemType_COUNT:
		printf(
			"%zu",
			quaditem->offset_node);
		break;
	default: assert(false);
	}

}

static void print_info_quadruple_entry(
const Parser* parser,
const QuadEntry* quadentry) {
	const char* type;

	switch(quadentry->op.type) {
	case QuadItemType_SCOPE_PAL: type = "SCOPE PAL"; break;
	case QuadItemType_SCOPE_END_PAL: type = "SCOPE END PAL"; break;
	case QuadItemType_MOVE: type = "MOVE"; break;
	case QuadItemType_ADD: type = "ADD"; break;
	case QuadItemType_SUB: type = "SUB"; break;
	case QuadItemType_MUL: type = "MUL"; break;
	case QuadItemType_DIV: type = "DIV"; break;
	case QuadItemType_ARG: type = "ARG"; break;
	case QuadItemType_CALL: type = "CALL"; break;
	case QuadItemType_PAL: type = "PAL"; break;
	case QuadItemType_COUNT: type = "COUNT"; break;
	default: assert(false);
	}

	const char* code = parser->lexer->source->content;
	const Node* nodes = parser->nodes.base;
	const Token* tokens = parser->lexer->tokens.base;

	const Node* node_src1 = nodes + quadentry->src1.offset_node;
	const Node* node_src2 = nodes + quadentry->src2.offset_node;
	const Node* node_dst = nodes + quadentry->dst.offset_node;

	const Token* token_src1 = tokens + node_src1->offset_token;
	const Token* token_src2 = tokens + node_src2->offset_token;
	const Token* token_dst = tokens + node_dst->offset_token;

	printf(
		"\t%s",
		type);
	print_info_quaditem(
		code,
		token_src1,
		&quadentry->src1);
	print_info_quaditem(
		code,
		token_src2,
		&quadentry->src2);
	print_info_quaditem(
		code,
		token_dst,
		&quadentry->dst);
	printf("\n");
}

void debug_print_quadruple_list(const TAC* tac) {
	printf("QUADRUPLES:\n");

	const QuadList* quadruple_list = &tac->quadlist;
	QuadEntry* base = quadruple_list->quadruples.area.base;
	size_t count = base == NULL
		? 0
		: (size_t)((const char*) quadruple_list->quadruples.top - (const char*) base) / sizeof(QuadEntry);

	for(
	size_t i = 1;
	i < count - 1;
	i += 1) {
		print_info_quadruple_entry(
			tac->stab.parser,
			base + i);
	}

	printf(
		"\nNumber of quadruplets: %zu\n",
		count - 2);
}

#endif
