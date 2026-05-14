#ifndef PARSER_H
#define PARSER_H

#include <assert.h>
#include <stdint.h>
#include "allocator.h"
#include "lexer.h"

typedef enum: uint64_t {
#define NODE_TYPE(type) NodeType_ ## type
	NODE_TYPE(NO) = 0,
// LIT
	NODE_TYPE(LIT_NUM),
	NODE_TYPE(LIT_CHR),
	NODE_TYPE(LIT_STR),
// Groups
	NODE_TYPE(GROUP_QUALIFIERS),
	NODE_TYPE(GROUP_PARENTHESIS),
	NODE_TYPE(GROUP_BRACKETS),
// Scope types
	NODE_TYPE(SCOPE),
	NODE_TYPE(SCOPE_IF),
	NODE_TYPE(SCOPE_ELSE_IF),
	NODE_TYPE(SCOPE_ELSE),
	NODE_TYPE(THROUGH),
	NODE_TYPE(THROUGH_NOT),
// Keys actions
	NODE_TYPE(DECL_VAR),
	NODE_TYPE(DECL_PAL),
	NODE_TYPE(CALL)
#undef NODE_TYPE
} NodeType;

typedef struct {
	NodeType type;
	uint32_t arity;
	long int token;
} Node;

typedef enum: uint64_t {
#define CONTEXT_TYPE(type) ContextType_ ## type
	CONTEXT_TYPE(NO) = 0,
	CONTEXT_TYPE(SCOPE_0),
#undef CONTEXT_TYPE
} ContextType;

typedef struct {
	ContextType type;
	uint32_t watermark;
	uint32_t child_count;
	size_t token;
} Context;

typedef struct {
	NodeType type;
	uint32_t precedence;
	uint32_t count_arity;
	size_t token;
} Operator;

typedef struct {
	const Lexer* lexer;
	MemoryArea nodes;
} Parser;

void initialize_parser(Parser* parser);
bool create_parser(
	const Lexer* lexer,
	Parser* parser);
void destroy_parser(Parser* parser);

#endif
