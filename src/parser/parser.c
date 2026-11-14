#include "parser.h"
#include "allocator.h"
#include "lexer.h"
#include "parser_declaration.h"
#include "parser_expression.h"
#include "parser_module.h"
#include "parser_node.h"
#include "parser_qualifier.h"
#include "parser_scope.h"
#include "parser_utils.h"
#include <stdio.h>

static int error = 0;

static int set_error(int value) {
	if(error == -1)
		return -1;

	error = value;
	return value;
}

static void emit_node_null(Node* token) {
	*token = (Node) {
		.type = NodeType_NO,
		.arity = 0,
		.offset_token = 0};
}

static bool parser_create_allocator_limit(
size_t limit,
Parser* parser) {
	if(create_memory_area(
		limit,
		sizeof(Node),
		&parser->nodes)
	== false)
		return false;

	emit_node_null((Node*) parser->nodes.base);
	return true;
}

static bool parser_allocator_shrink_append_null(Parser* parser) {
	assert(parser->nodes.count <= (size_t) parser->lexer->source->length);

	if(memory_area_realloc(
		parser->nodes.count + 1, // null token
		&parser->nodes)
	== false)
		return false;
	// sentinel
	emit_node_null((Node*) parser->nodes.base + parser->nodes.count - 1);
	return true;
}

static void parser_destroy_allocator(Parser* parser) {
	if(parser == NULL)
		return;

	destroy_memory_area(&parser->nodes);
}

bool create_parser(
const Lexer* lexer,
Parser* parser) {
	assert(lexer != NULL);
	assert(parser != NULL);

	error = 0;

	parser->lexer = lexer;

	auto stack_context = setup_memory_stack();
	auto stack_operator = setup_memory_stack();
	auto stack_buffer = setup_memory_stack();

	if(!create_memory_stack(
		1024,
		sizeof(Context),
		&stack_context)
	|| !create_memory_stack(
		1024,
		sizeof(Operator),
		&stack_operator)
	|| create_memory_stack(
		1024,
		sizeof(Operator),
		&stack_buffer)
	== false) {
		set_error(-1);
		goto CLEAR;
	}	

	Context context_scope_0 = (Context) {
		.type = ContextType_SCOPE_0,
		.watermark = 0,
		.count_child = 0,
		.token = 0};
	memory_stack_push(
		(char*) &context_scope_0,
		&stack_context);
	
	if(parser_create_allocator_limit(
		lexer->source->length,
		parser)
	== false) {
		set_error(-1);
		goto CLEAR;
	}

	const Token* const tokens = lexer->tokens.base;
	size_t i = 1; // token position
	size_t j = 1; // node position

	while(i < lexer->tokens.count - 1) {
		if(if_GRP_Q_emit_operator(
			&i,
			&j,
			&stack_context,
			&stack_operator,
			parser)
		== true) {
			continue;
		} else if(if_LSCOPE_push_context(
			&i,
			&stack_context,
			&stack_operator,
			parser)
		== true) {
			continue; // the comma is optional
		} else if(if_EXP_emit_operator(
			&i,
			&j,
			&stack_operator,
			&stack_buffer,
			parser)
		== true) {
			// OK
		} else if(if_DECL_emit_operator(
			&i,
			&j,
			&stack_context,
			&stack_operator,
			&stack_buffer,
			parser)
		== true) {
			Operator* top_operator = memory_stack_top_addr(&stack_operator);

			if(top_operator->type == NodeType_INIT_LAB
			|| top_operator->type == NodeType_INIT_PAL)
				continue; // skip to the next instruction
		} else if(if_MOD_emit_operator(
			&i,
			&j,
			&stack_operator,
			parser)
		== true) {
			// OK
		}

		if(parser_is_instruction_end(tokens + i)) {
			// handle `;`
			// flush the instruction
			parser_context_flush(
				&j,
				&stack_context,
				&stack_operator,
				parser);
			// ignore all the `;`
			while(parser_is_instruction_end(tokens + i))
				i += 1;
		} else if(if_LSCOPE_end_pop_context(
			&i,
			&j,
			&stack_context,
			&stack_operator,
			parser)
		== true) {
			// handle `.`
		} else {
			set_error(-1);
			break;
		}
	}

	if(i == 1)
		set_error(-1); // do a better error code later
	// the last context must be the first one (prevents not ended scopes)
	Context* top_context = memory_stack_top_addr(&stack_context);

	if(top_context->type != ContextType_SCOPE_0)
		set_error(-1);

	parser->nodes.count = j;

	if(!parser_allocator_shrink_append_null(parser))
		set_error(-1);
CLEAR:
	destroy_memory_stack(&stack_buffer);
	destroy_memory_stack(&stack_operator);
	destroy_memory_stack(&stack_context);

	if(error == 0)
		return true;

	destroy_parser(parser);
	return false;
}

void destroy_parser(Parser* parser) {
	if(parser == NULL)
		return;

	parser_destroy_allocator(parser);
	*parser = setup_parser();
}

#undef DEFINE_OPERATOR
#undef DEFINE_CONTEXT
