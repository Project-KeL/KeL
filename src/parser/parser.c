#include "parser.h"
#include "allocator.h"
#include "lexer.h"
#include "parser_allocator.h"
#include "parser_declaration.h"
#include "parser_node.h"
#include "parser_scope.h"
#include "parser_utils.h"
#include <stdio.h>

/*
 * GRP_Q
 * DECL: VAR or PAL (consumes the identifier, the type and, if it exists, the initialization)
*/

static int error = 0;

static int set_error(int value) {
	if(error == -1)
		return -1;

	error = value;
	return value;
}

static void if_GRP_Q_create_operator(
size_t* i,
size_t* j,
MemoryStack* stack_operator,
Parser* parser) {
	const Token* const tokens = parser->lexer->tokens.base;

	if(!parser_is_Q(tokens + *i))
		return;

	Operator operator = (Operator) {
		.type = NodeType_GRP_Q,
		.precedence = 0,
		.count_arity = 0,
		.token = *i};
	memory_stack_push(
		(char*) &operator,
		stack_operator);

	do {
		parser_create_leaf(
			NodeType_Q,
			*i,
			j,
			stack_operator,
			parser);
		*i += 1;
	} while(parser_is_Q(tokens + *i));

	Operator pop_operator;
	memory_stack_pop(
		(char*) &pop_operator,
		stack_operator);
	parser_create_operator(
		pop_operator.type,
		pop_operator.count_arity,
		pop_operator.token,
		j,
		stack_operator,
		parser);
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
	initialize_memory_stack(&stack_context);
	initialize_memory_stack(&stack_operator);

	if(!create_memory_stack(
		1024,
		sizeof(Context),
		&stack_context)
	|| create_memory_stack(
		1024,
		sizeof(Operator),
		&stack_operator)
	== false) {
		set_error(-1);
		goto CLEAR;
	}

	Context context;
	context = (Context) {
		.type = ContextType_SCOPE_0,
		.watermark = 0,
		.count_child = 0,
		.token = 0};
	memory_stack_push(
		(char*) &context,
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

	while(i < lexer->tokens.count - 1) {
		if_GRP_Q_create_operator(
			&i,
			&j,
			&stack_operator,
			parser);

		if(if_LSCOPE_create_context(
			&i,
			&stack_context,
			&stack_operator,
			parser)
		== true) {
			continue;
		} else if(if_DECL_create_operator(
			&i,
			&j,
			&stack_context,
			&stack_operator,
			parser)
		== true) {
			Operator* top_operator = memory_stack_top_addr(&stack_operator);

			if(top_operator->type == NodeType_INIT_PAL)
				continue;
		}
	
		if(parser_is_instruction_end(tokens + i)) {
			Operator pop_operator;
			memory_stack_pop(
				(char*) &pop_operator,
				&stack_operator);
			parser_create_operator(
				pop_operator.type,
				pop_operator.count_arity,
				pop_operator.token,
				&j,
				&stack_operator,
				parser);
			Context* top_context = memory_stack_top_addr(&stack_context);
			top_context->count_child += 1;
			// ignore all the `;`
			while(parser_is_instruction_end(tokens + i))
				i += 1;
		} else if(if_LSCOPE_end_destroy_context(
			&i,
			&j,
			&stack_context,
			&stack_operator,
			parser)
		== true) {
		} else {
			set_error(-1);
			break;
		}
	}

	if(i == 0)
		set_error(-1); // do a better error code later
	// the last context must be the first one (prevents not ended scopes)
	Context* top_context = memory_stack_top_addr(&stack_context);

	if(top_context->type != ContextType_SCOPE_0)
		set_error(-1);

	parser->nodes.count = j;

	if(!parser_allocator_shrink(parser))
		set_error(-1);
CLEAR:
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
