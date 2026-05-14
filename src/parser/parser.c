#include "parser.h"
#include "allocator.h"
#include "parser_allocator.h"

#define DEFINE_CONTEXT(context) Context* context = (Context*) stack_context.addr + stack_context.count
#define DEFINE_OPERATOR(operator) Operator operator = (Operator) stack_operator.addr + stack_operator.count

static int error;

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

	MemoryArea stack_context;
	MemoryArea stack_operator;
	initialize_memory_area(&stack_context);
	initialize_memory_area(&stack_operator);

	if(!create_memory_area(
		1024,
		sizeof(Context),
		&stack_context)
	|| create_memory_area(
		1024,
		sizeof(Operator),
		&stack_operator)
	== false) {
		error = -1;
		goto CLEAR;
	}

	DEFINE_CONTEXT(context);
	*context = (Context) {
		.type = ContextType_SCOPE_0,
		.watermark = 0,
		.child_count = 0,
		.token = 0};

	parser_initialize_allocator(parser);

	if(parser_create_allocator_limit(
		lexer->source->length,
		parser)
	== false) {
		error = -1;
		goto CLEAR;
	}

CLEAR:
	destroy_memory_area(&stack_operator);
	destroy_memory_area(&stack_context);

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
