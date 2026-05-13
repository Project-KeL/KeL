#ifndef PARSER_H
#define PARSER_H

#include <assert.h>
#include <stdint.h>
#include "allocator.h"
#include "lexer.h"
#include "parser_def.h"

typedef enum: uint64_t {
#define NODE_TYPE(type) NodeType_ ## type
	NODE_TYPE(NO) = 0,
// Scope types
	NODE_TYPE(SCOPE),
	NODE_TYPE(SCOPE_IF),
	NODE_TYPE(SCOPE_ELSE_IF),
	NODE_TYPE(SCOPE_ELSE),
	NODE_TYPE(THROUGH),
	NODE_TYPE(THORUGH_NOT),
// Expression
	NODE_TYPE(TEMPORARY),
#undef NODE_TYPE
} NodeType;

typedef struct {
	NodeType type;
	/*
	union {
		struct {}
	};	
	*/
} Node;

typedef struct {
	const Lexer* lexer;
	MemoryChain nodes;
} Parser;

void initialize_parser(Parser* parser);
bool create_parser(
	const Lexer* lexer,
	MemoryArea* restrict memArea,
	Parser* parser);
void destroy_parser(Parser* parser);

#endif
