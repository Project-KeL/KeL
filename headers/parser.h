#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include "allocator.h"
#include "lexer.h"

#define MASK_NODE_TYPE 0xFFFF000000000000

typedef enum: uint64_t {
#define NODE_TYPE(type) NodeType_ ## type
#define VALUE(value) ((uint64_t) value << 48) // two bytes for the type of the nodes
	NODE_TYPE(NO) = 0,
	NODE_TYPE(SCOPE_START) = VALUE(0x0001),
	NODE_TYPE(SCOPE_END) = VALUE(0x0002),
	NODE_TYPE(DECLARATION) = VALUE(0x0003),
	NODE_TYPE(INITIALIZATION) = VALUE(0x0004),
	NODE_TYPE(AFFECTATION) = VALUE(0x0005),
	NODE_TYPE(EXPRESSION) = VALUE(0x0006),
#undef VALUE
#undef NODE_TYPE
} NodeType;

typedef enum: uint64_t {
#define NODE_TYPE_SCOPE(type) NodeTypeScope_ ## type
	NODE_TYPE_SCOPE(NO) = 0,
	NODE_TYPE_SCOPE(THEN),
	NODE_TYPE_SCOPE(THEN_NOT),
	NODE_TYPE_SCOPE(THROUGH),
	NODE_TYPE_SCOPE(THROUGH_NOT),
	NODE_TYPE_SCOPE(TEST),
#undef NODE_TYPE_SCOPE
} NodeTypeScope;

typedef struct {
	uint64_t type;
	union {
		uint64_t value; // can be used as a subtype
		void* value_pointer;
		void (*value_function)();
		Token* token;};
	union {
		const struct Node* child;
		struct {
			const struct Node* child1;
			const struct Node* child2;};};
} Node;

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
