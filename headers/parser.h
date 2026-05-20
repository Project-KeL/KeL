#ifndef PARSER_H
#define PARSER_H

#include <assert.h>
#include <stdint.h>
#include "allocator.h"
#include "lexer.h"

typedef enum: uint64_t {
#define NODE_TYPE(type) NodeType_ ## type
	NODE_TYPE(NO) = 0,
// Qualifiers
	NODE_TYPE(Q),
// LIT
	NODE_TYPE(LIT_NUM),
	NODE_TYPE(LIT_CHAR),
	NODE_TYPE(LIT_STR),
// Groups
	NODE_TYPE(GRP_Q),
	NODE_TYPE(GRP_L_PARES),
	NODE_TYPE(GRP_R_PARES),
	NODE_TYPE(GRP_BRACS),
// Scope types
	NODE_TYPE(SCOPE),
	NODE_TYPE(SCOPE_IF),
	NODE_TYPE(SCOPE_THROUGH),
	NODE_TYPE(SCOPE_ELSE_IF),
	NODE_TYPE(SCOPE_ELSE),
	NODE_TYPE(SCOPE_ELSE_THROUGH),
// Keys actions
	NODE_TYPE(DECL_VAR),
	NODE_TYPE(DECL_PAL),
	NODE_TYPE(INIT_VAR),
	NODE_TYPE(INIT_PAL),
	NODE_TYPE(TYPE_VAR),
	NODE_TYPE(TYPE_PAL), // TYPE_VAR + GRP_PARES
	NODE_TYPE(TYPE_PAL_VOID), // is implicit (PAL with no return type or empty parenthesis)
	NODE_TYPE(ID),
	NODE_TYPE(L),
	NODE_TYPE(PARAM),
	NODE_TYPE(CALL),
// Operators
	NODE_TYPE(ADD),
	NODE_TYPE(SUB),
	NODE_TYPE(MUL),
	NODE_TYPE(DIV),
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
	CONTEXT_TYPE(SCOPE),
	CONTEXT_TYPE(SCOPE_IF),
	CONTEXT_TYPE(SCOPE_THROUGH),
	CONTEXT_TYPE(SCOPE_ELSE_IF),
	CONTEXT_TYPE(SCOPE_ELSE),
#undef CONTEXT_TYPE
} ContextType;

typedef struct {
	ContextType type;
	uint32_t watermark;
	uint32_t count_child;
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
