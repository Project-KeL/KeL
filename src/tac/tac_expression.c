#include <assert.h>
#include "lexer.h"
#include "tac_expression.h"
#include "parser.h"
#include "parser_utils.h"
#include "tac_stab.h"
#include "tac_quadruple.h"
#include <stdio.h>

static QuadItem get_operand(
const Node* operand,
TAC* tac) {
	Node* nodes = tac->stab.parser->nodes.base;

	switch(operand->type) {
	case NodeType_LIT_NUM:
	case NodeType_LIT_CHAR:
	case NodeType_LIT_STR:
		return (QuadItem) {
			.offset_node = (size_t)(operand - nodes),
			.type = QuadItemType_LIT};
	case NodeType_ID:
	case NodeType_KEY:
		STabEntry* entry = tac_stab_lookup(
			(size_t)(operand - nodes),
			&tac->stab);
		assert(entry != NULL);
		return (QuadItem) {
			.offset_node = entry->offset_node,
			.type = QuadItemType_KEY};
	case NodeType_OP_ADD:
	case NodeType_OP_SUB:
	case NodeType_OP_MUL:
	case NodeType_OP_DIV:
	case NodeType_CALLEE:
		return (QuadItem) {
			.offset_node = (size_t)(operand - nodes),
			.type = QuadItemType_TEMP};
	default: assert(false);
	}
}

void tac_create_expression(
size_t start,
size_t end,
size_t dst,
MemoryStack* stack_buffer,
TAC* tac) {
	const Parser* parser = tac->stab.parser;
	const Token* tokens = parser->lexer->tokens.base;

	for(
	size_t i = start;
	i < end;
	i += 1) {
		Node* node = (Node*) parser->nodes.base + i;
		const Token* token = tokens + node->offset_token;

		if(node->type == NodeType_CALLEE) {
			for(
			size_t j = 0;
			j < node->arity;
			j += 1) {
				Node* node_arg;
				memory_stack_pop(
					(char*) &node_arg,
					stack_buffer);

				QuadEntry entry_arg = (QuadEntry) {
					.op = (QuadItem) {
						.type = QuadItemType_ARG,
						.offset_node = i},
					.src1 = get_operand(
						node_arg,
						tac),
					.src2 = create_quaditem_null(),
					.dst = create_quaditem_null()};
				quadlist_append(
					&entry_arg,
					&tac->quadlist);
			}

			QuadEntry entry_call = (QuadEntry) {
				.op = (QuadItem) {
					.type = QuadItemType_CALL,
					.offset_node = i},
				.src1 = (QuadItem) {
					.type = QuadItemType_PAL,
					.offset_node = i},
				.src2 = (QuadItem) {
					.type = QuadItemType_COUNT,
					.offset_node = node->arity},
				.dst = (QuadItem) {
					.type = QuadItemType_TEMP,
					.offset_node = i}};
			quadlist_append(
				&entry_call,
				&tac->quadlist);
			memory_stack_push(
				(char*) &node,
				stack_buffer);
		} else if(node->arity == 0) {
			memory_stack_push(
				(char*) &node,
				stack_buffer);
		} else if(parser_is_operator_algebraic(token)) {
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
			QuadEntry quadentry;

			if(node->type == NodeType_OP_ASSIGN) {
				quadentry = (QuadEntry) {
					.op = (QuadItem) {
						.type = QuadItemType_MOVE,
						.offset_node = i},
					.src1 = get_operand(
						left,
						tac),
					.src2 = create_quaditem_null(),
					.dst = get_operand(
						right,
						tac)};
				node = right; // WARNING
			} else {
				QuadItemType op_type;

				switch(node->type) {
				case NodeType_OP_ADD: op_type = QuadItemType_ADD; break;
				case NodeType_OP_SUB: op_type = QuadItemType_SUB; break;
				case NodeType_OP_MUL: op_type = QuadItemType_MUL; break;
				case NodeType_OP_DIV: op_type = QuadItemType_DIV; break;
				default: assert(false);
				}

				quadentry = (QuadEntry) {
					.op = (QuadItem) {
						.type = op_type,
						.offset_node = i},
					.src1 = get_operand(
						left,
						tac),
					.src2 = get_operand(
						right,
						tac),
					.dst = (QuadItem) {
						.type = QuadItemType_TEMP,
						.offset_node = i}};
			}

			quadlist_append(
				&quadentry,
				&tac->quadlist);
			memory_stack_push(
				(char*) &node,
				stack_buffer);
		}  else
				assert(false);
	}

	if(dst != 0) {
		Node* result;
		memory_stack_pop(
			(char*) &result,
			stack_buffer);

		QuadEntry quadentry = (QuadEntry) {
			.op = (QuadItem) {
				.type = QuadItemType_MOVE,
				.offset_node = dst},
			.src1 = get_operand(
				result,
				tac),
			.src2 = create_quaditem_null(),
			.dst = get_operand(
				(Node*) parser->nodes.base + dst,
				tac)};
		quadlist_append(
			&quadentry,
			&tac->quadlist);
	}
}
