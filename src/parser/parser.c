#include "parser.h"
#include "allocator.h"
#include "lexer.h"
#include "parser_allocator.h"
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

void initialize_parser(Parser* parser) {
	assert(parser != NULL);

	parser->lexer = NULL;
	initialize_memory_area(&parser->nodes);
}

bool create_parser(
const Lexer* lexer,
Parser* parser) {
	assert(lexer != NULL);
	assert(parser != NULL);

	error = 0;

	parser->lexer = lexer;

	MemoryStack stack_context;
	MemoryStack stack_operator;
	MemoryStack stack_buffer;
	initialize_memory_stack(&stack_context);
	initialize_memory_stack(&stack_operator);
	initialize_memory_stack(&stack_buffer);

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
	parser_initialize_allocator(parser);
	
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
	size_t i_Q = 0; // qualifiers

	while(i < lexer->tokens.count - 1) {
		if(i_Q == 0
		&& parser_is_Q(tokens + i)) {
			i_Q = i; // keep the track of the position
			while(parser_is_Q(tokens + i)) i += 1; // reserve qualifiers for instructions end
		}

		if(if_LSCOPE_create_context(
			&i,
			&stack_context,
			&stack_operator,
			parser)
		== true) {
			continue; // the comma is optional
		} else if(if_EXP_create_operator(
			&i,
			&j,
			&stack_operator,
			&stack_buffer,
			parser)
		== true) {
			// OK
		} else if(if_DECL_create_operator(
			&i,
			&j,
			&stack_context,
			&stack_operator,
			&stack_buffer,
			parser)
		== true) {
			Operator* top_operator = memory_stack_top_addr(&stack_operator);

			if(top_operator->type == NodeType_INIT_PAL)
				continue; // skip to the next instruction
		} else if(if_MOD_create_operator(
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
			Context* top_context = memory_stack_top_addr(&stack_context);
		
			if(i_Q != 0
			&& i_Q > top_context->token) {
				// place qualifiers at the end of the RPN
				if_GRP_Q_create_operator(
					&j,
					i_Q,
					&stack_context,
					&stack_operator,
					parser);
				i_Q = 0;
			}
			// ignore all the `;`
			while(parser_is_instruction_end(tokens + i))
				i += 1;
		} else if(if_LSCOPE_end_destroy_context(
			&i,
			&j,
			&i_Q,
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
	initialize_parser(parser);
}

#undef DEFINE_OPERATOR
#undef DEFINE_CONTEXT
