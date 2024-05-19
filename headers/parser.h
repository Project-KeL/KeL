#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include "allocator.h"
#include "lexer.h"

typedef enum: uint64_t {
#define NODE_TYPE(type) NodeType_ ## type
	NODE_TYPE(NO) = 0,
	NODE_TYPE(CHILD),
	NODE_TYPE(SCOPE_START), // `.child` holds the ending scope node
	NODE_TYPE(SCOPE_END),
	NODE_TYPE(DECLARATION),
	NODE_TYPE(INITIALIZATION),
	NODE_TYPE(AFFECTATION),
	NODE_TYPE(EXPRESSION),
#undef NODE_TYPE
} NodeType;

typedef enum: uint64_t {
#define NODE_SUBTYPE(subtype) NodeSubtype_ ## subtype
	NODE_SUBTYPE(NO) = 0,
#undef NODE_SUBTYPE
} NodeSubtype;

typedef enum: uint64_t {
#define NODE_SUBTYPE_SCOPE(type) NodeSubtypeScope_ ## type
	NODE_SUBTYPE_SCOPE(NO) = 0,
	NODE_SUBTYPE_SCOPE(THEN),
	NODE_SUBTYPE_SCOPE(THEN_NOT),
	NODE_SUBTYPE_SCOPE(THROUGH),
	NODE_SUBTYPE_SCOPE(THROUGH_NOT),
	NODE_SUBTYPE_SCOPE(TEST),
#undef NODE_SUBTYPE_SCOPE
} NodeSubtypeScope;

typedef enum: uint64_t {
#define NODE_SUBTYPE_KEY_QUALIFICATION(subtype) NodeSubtypeKeyQualification_ ## subtype
	NODE_SUBTYPE_KEY_QUALIFICATION(NO) = 0,
	NODE_SUBTYPE_KEY_QUALIFICATION(MUT),
#undef NODE_SUBTYPE_KEY_QUALIFICATION
} NodeSubtypeKeyQualification;

/*
 * `LEFT` and `RIGHT` is relative to the lock
 * if an array has a bound, it is set in `.value`
*/

typedef enum: uint64_t {
#define NODE_SUBTYPE_CHILD_KEY_TYPE(subtype) NodeSubtypeChildKeyType_ ## subtype
	NODE_SUBTYPE_CHILD_KEY_TYPE(NO) = 0,
	NODE_SUBTYPE_CHILD_KEY_TYPE(AMPERSAND_LEFT),
	NODE_SUBTYPE_CHILD_KEY_TYPE(AMPERSAND_RIGHT),
	NODE_SUBTYPE_CHILD_KEY_TYPE(ARRAY), // brackets are always before the lock
	NODE_SUBTYPE_CHILD_KEY_TYPE(ARRAY_BOUND),
	NODE_SUBTYPE_CHILD_KEY_TYPE(MINUS_LEFT),
	NODE_SUBTYPE_CHILD_KEY_TYPE(MINUS_RIGHT),
	NODE_SUBTYPE_CHILD_KEY_TYPE(PIPE_LEFT),
	NODE_SUBTYPE_CHILD_KEY_TYPE(PIPE_RIGHT),
	NODE_SUBTYPE_CHILD_KEY_TYPE(PLUS_LEFT),
	NODE_SUBTYPE_CHILD_KEY_TYPE(PLUS_RIGHT),
#undef NODE_SUBTYPE_CHILD_KEY_TYPE
} NodeSubtypeChildKeyType;

typedef struct Node Node;

struct Node {
	uint64_t type;
	uint64_t subtype;
	union {
		uint64_t value;
		void* value_pointer;
		void (*value_function)();
		const Token* token;};
	union {
		Node* child;
		struct {
			Node* child1;
			Node* child2;};};
};

typedef struct {
	const Lexer* lexer;
	Node* nodes;
	long int count;
} Parser;

bool create_parser(
	const Lexer* lexer,
	Allocator* restrict allocator,
	Parser* restrict parser);
void destroy_parser(Parser* restrict parser);

#endif
