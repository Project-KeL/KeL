#ifndef PARSER_H
#define PARSER_H

#include <assert.h>
#include <stdint.h>
#include "allocator.h"
#include "lexer.h"

typedef enum: uint64_t {
#define NODE_TYPE(type) NodeType_ ## type
	NODE_TYPE(NO) = 0,
// Q
	NODE_TYPE(Q),
// LIT
	NODE_TYPE(LIT_NUM),
	NODE_TYPE(LIT_CHAR),
	NODE_TYPE(LIT_STR),
// GRP
	NODE_TYPE(GRP_IMOD),
	NODE_TYPE(GRP_OMOD),
	NODE_TYPE(GRP_Q),
	NODE_TYPE(GRP_L_PARES),
	NODE_TYPE(GRP_R_PARES),
	NODE_TYPE(GRP_BRAC),
	NODE_TYPE(GRP_CALL_ARGS),
// MOD
	NODE_TYPE(IMOD),
	NODE_TYPE(OMOD),
// Scope types
	NODE_TYPE(SCOPE),
	NODE_TYPE(SCOPE_IF),
	NODE_TYPE(SCOPE_THROUGH),
	NODE_TYPE(SCOPE_ELSE_IF),
	NODE_TYPE(SCOPE_ELSE),
	NODE_TYPE(SCOPE_ELSE_THROUGH),
	NODE_TYPE(SCOPE_END), // last child of a scope, to ease the TAC
// EXP
	NODE_TYPE(EXP),
// Keys actions
	NODE_TYPE(DECL_VAR),
	NODE_TYPE(DECL_LAB),
	NODE_TYPE(DECL_PAL),
	NODE_TYPE(INIT_VAR),
	NODE_TYPE(INIT_LAB),
	NODE_TYPE(INIT_PAL),
	NODE_TYPE(TYPE_VAR),
	NODE_TYPE(TYPE_PAL), // TYPE_VAR + GRP_PARES
	NODE_TYPE(TYPE_PAL_VOID), // is implicit (PAL with no return type or empty parenthesis)
	NODE_TYPE(ID), // for declarations
	NODE_TYPE(KEY), // an L without subtype
	NODE_TYPE(PARAM),
	NODE_TYPE(CALLEE),
// OP
	NODE_TYPE(OP_ASSIGN),
	NODE_TYPE(OP_LPARENTHESIS),
	NODE_TYPE(OP_ADD),
	NODE_TYPE(OP_SUB),
	NODE_TYPE(OP_MUL),
	NODE_TYPE(OP_DIV),
	NODE_TYPE(OP_CALL),
#undef NODE_TYPE
} NodeType;

typedef struct {
	NodeType type;
	uint32_t arity;
	long int offset_token;
} Node;

typedef enum: uint64_t {
#define CONTEXT_TYPE(type) ContextType_ ## type
	CONTEXT_TYPE(NO) = 0,
// Scope
	CONTEXT_TYPE(SCOPE_0),
	CONTEXT_TYPE(SCOPE),
	CONTEXT_TYPE(SCOPE_IF),
	CONTEXT_TYPE(SCOPE_ELSE_IF),
	CONTEXT_TYPE(SCOPE_ELSE),
	CONTEXT_TYPE(SCOPE_THROUGH),
	CONTEXT_TYPE(SCOPE_THROUGH_NOT),
// Key
	CONTEXT_TYPE(EXP),
// Call
	CONTEXT_TYPE(CALL),
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
	size_t offset_token;
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
