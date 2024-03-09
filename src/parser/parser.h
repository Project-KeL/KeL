#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include "../lexer/lexer.h"

typedef enum: uint32_t {
#define NODE_TYPE(type) NodeType_ ## type
	NODE_TYPE(NO) = 0,
	// core
	NODE_TYPE(CORE_B),
	NODE_TYPE(CORE_W),
	NODE_TYPE(CORE_D),
	NODE_TYPE(CORE_Q),
#undef NODE_TYPE
} NodeType;

typedef struct Node {
	NodeType type;
	union {
		uint64_t value;
		void* valueptr;};
	struct Node* child1;
	struct Node* child2;
} Node;

typedef struct {
	Lexer* lexer;
	Node* nodes;
	long int count;
} Parser;

bool create_parser(
	Lexer* lexer,
	Parser* restrict parser);
void destroy_parser(Parser* restrict parser);

#endif
