#include <assert.h>
#include "lexer.h"
#include "tac_expression.h"
#include "parser.h"
#include "tac_quadruple.h"
#include <stdio.h>

static QuadrupleItemType get_operand(const Node* operand) {
	switch(operand->type) {
	case NodeType_LIT_NUM:
	case NodeType_LIT_CHAR:
	case NodeType_LIT_STR:
		return QuadrupleItemType_LIT;
	case NodeType_OP_ADD:
	case NodeType_OP_SUB:
	case NodeType_OP_MUL:
	case NodeType_OP_DIV:
		return QuadrupleItemType_TEMP;
	default: assert(false);
	}
}

void tac_create_expression(
size_t start,
size_t end,
MemoryStack* stack_buffer,
TAC* tac) {
	const Parser* parser = tac->stab.parser;
	const Node* nodes = parser->nodes.base;

	for(
	size_t i = start;
	i < end;
	i += 1) {
		Node* node = (Node*) parser->nodes.base + i;

		if(node->arity == 0) {
			memory_stack_push(
				(char*) &node,
				stack_buffer);
		} else if(node->arity == 2) {
			Node* right;
			Node* left;
			memory_stack_pop(
				(char*) &right,
				stack_buffer);
			memory_stack_pop(
				(char*) &left,
				stack_buffer);

			const Token* tokens = parser->lexer->tokens.base;
			[[maybe_unused]] const Token* token_left = tokens + left->offset_token;
			[[maybe_unused]] const Token* token_right = tokens + right->offset_token;
			QuadrupleItemType op_type;

			switch(node->type) {
			case NodeType_OP_ADD: op_type = QuadrupleItemType_ADD; break;
			case NodeType_OP_SUB: op_type = QuadrupleItemType_SUB; break;
			case NodeType_OP_MUL: op_type = QuadrupleItemType_MUL; break;
			case NodeType_OP_DIV: op_type = QuadrupleItemType_DIV; break;
			default: assert(false);
			}

			QuadrupleEntry quadruple_entry = (QuadrupleEntry) {
				.op = (QuadrupleItem) {
					.type = op_type,
					.offset_node = i},
				.src1 = (QuadrupleItem) {
					.type = get_operand(left),
					.offset_node = (size_t)(left - nodes)},
				.src2 = (QuadrupleItem) {
					.type = get_operand(right),
					.offset_node = (size_t)(right - nodes)},
				.dst = (QuadrupleItem) {
					.type = QuadrupleItemType_TEMP,
					.offset_node = i}};

			quadruple_list_append(
				&quadruple_entry,
				&tac->quadruple_list);
			memory_stack_push(
				(char*) &node,
				stack_buffer);
			}  else
				assert(false);
	}
	printf("\n");
}
