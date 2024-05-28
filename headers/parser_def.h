#ifndef PARSER_DEF
#define PARSER_DEF

#include <stdint.h>
#include "lexer_def.h"

typedef enum: uint64_t {
#define NODE_TYPE(type) NodeType_ ## type
	NODE_TYPE(NO) = 0,
	NODE_TYPE(SCOPE_START), // `.child` holds the ending scope node
	NODE_TYPE(SCOPE_END),
	NODE_TYPE(QUALIFIER),
	NODE_TYPE(IDENTIFICATION), // `.subtype` holds the command, the type of identification and the qualifiers
	NODE_TYPE(LITERAL),
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
#define NODE_TYPE_CHILD(type) NodeTypeChild_ ## type
	NODE_TYPE_CHILD(NO) = 0,
#undef NODE_TYPE_CHILD_TYPE
} NodeTypeChild;

typedef enum: uint64_t {
#define NODE_SUBTYPE_CHILD(subtype) NodeSubtypeChild_ ## subtype
	NODE_SUBTYPE_CHILD(NO) = 0,
#undef NODE_SUBTYPE_CHILD
} NodeSubtypeChild;

/*
 * SCOPE
*/

typedef enum: uint64_t {
#define NODE_SUBTYPE(type) NodeSubtypeScope_ ## type
	NODE_SUBTYPE(NO) = 0,
	NODE_SUBTYPE(THEN),
	NODE_SUBTYPE(THEN_NOT),
	NODE_SUBTYPE(THROUGH),
	NODE_SUBTYPE(THROUGH_NOT),
	NODE_SUBTYPE(TEST),
#undef NODE_SUBTYPE
} NodeSubtypeScope;

/*
 * IDENTIFICATION
*/

// These are flags (the full token subtype is useless so we extract only the qualifier part)
// The shift let the first bit empty to fetch the command
#define MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_COMMAND 0b1

typedef enum: uint64_t {
#define NODE_SUBTYPE_IDENTIFICATION(subtype) NodeSubtypeIdentificationBitCommand_ ## subtype
	NODE_SUBTYPE_IDENTIFICATION(HASH) = 0b0,
	NODE_SUBTYPE_IDENTIFICATION(AT) = 0b1,
#undef NODE_SUBTYPE_IDENTIFICATION
} NodeSubtypeIdentificationBitCommand;

#define SHIFT_NODE_SUBTYPE_IDENTIFICATION_BIT_TYPE 1
#define MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_TYPE (1 << SHIFT_NODE_SUBTYPE_IDENTIFICATION_BIT_TYPE)

typedef enum: uint64_t {
#define NODE_SUBTYPE_IDENTIFICATION(subtype) NodeSubtypeIdentificationBitType_ ## subtype
	NODE_SUBTYPE_IDENTIFICATION(DECLARATION) = 0b0 << SHIFT_NODE_SUBTYPE_IDENTIFICATION_BIT_TYPE,
	NODE_SUBTYPE_IDENTIFICATION(INITIALIZATION) = 0b1 << SHIFT_NODE_SUBTYPE_IDENTIFICATION_BIT_TYPE,
#undef NODE_SUBTYPE_IDENTIFICATION
} NodeSubtypeIdentificationBitType;

#define SHIFT_NODE_SUBTYPE_IDENTIFICATION_BIT_SCOPE 2
#define MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_SCOPE (1 << SHIFT_NODE_SUBTYPE_IDENTIFICATION_BIT_SCOPE)

typedef enum: uint64_t {
#define NODE_SUBTYPE_IDENTIFICATION(subtype) NodeSubtypeIdentificationBitScoped_ ## subtype
	NODE_SUBTYPE_IDENTIFICATION(FALSE) = 0,
	NODE_SUBTYPE_IDENTIFICATION(TRUE) = 1,
#undef NODE_SUBTYPE_IDENTIFICATION
} NodeSubtypeIdentificationBitScoped;

/*
 * `LEFT` and `RIGHT` is relative to the lock
 * if an array has a bound, it is set in `.value`
*/

typedef enum: uint64_t {
#define NODE_TYPE_CHILD_TYPE(type) NodeTypeChildType_ ## type
	NODE_TYPE_CHILD_TYPE(NO) = 0,
	NODE_TYPE_CHILD_TYPE(MODIFIER),
	NODE_TYPE_CHILD_TYPE(LOCK),
#undef NODE_TYPE_CHILD_TYPE
} NodeTypeChildType;

typedef enum: uint64_t {
#define NODE_SUBTYPE_CHILD_TYPE(subtype) NodeSubtypeChildTypeModifier_ ## subtype
	NODE_SUBTYPE_CHILD_TYPE(NO) = 0,
	NODE_SUBTYPE_CHILD_TYPE(AMPERSAND_LEFT),
	NODE_SUBTYPE_CHILD_TYPE(AMPERSAND_RIGHT),
	NODE_SUBTYPE_CHILD_TYPE(ARRAY), // brackets are always before the lock
	NODE_SUBTYPE_CHILD_TYPE(ARRAY_BOUND),
	NODE_SUBTYPE_CHILD_TYPE(MINUS_LEFT),
	NODE_SUBTYPE_CHILD_TYPE(MINUS_RIGHT),
	NODE_SUBTYPE_CHILD_TYPE(PIPE_LEFT),
	NODE_SUBTYPE_CHILD_TYPE(PIPE_RIGHT),
	NODE_SUBTYPE_CHILD_TYPE(PLUS_LEFT),
	NODE_SUBTYPE_CHILD_TYPE(PLUS_RIGHT),
#undef NODE_SUBTYPE_CHILD_TYPE
} NodeSubtypeChildTypeModifier;

typedef enum: uint64_t {
#define NODE_SUBTYPE_CHILD_TYPE(subtype) NodeSubtypeChildTypeScoped_ ## subtype
	NODE_SUBTYPE_CHILD_TYPE(RETURN_NONE) = 1,
	NODE_SUBTYPE_CHILD_TYPE(RETURN_LOCK),
	NODE_SUBTYPE_CHILD_TYPE(RETURN_SCOPE_START),
	NODE_SUBTYPE_CHILD_TYPE(RETURN_SCOPE_END),
	NODE_SUBTYPE_CHILD_TYPE(PARAMETER_NONE),
	NODE_SUBTYPE_CHILD_TYPE(PARAMETER),
	NODE_SUBTYPE_CHILD_TYPE(PARAMETER_LOCK),
#undef NODE_SUBTYPE_CHILD_TYPE
} NodeSubtypeChildTypeLock;

/*
 * LITERAL
*/

typedef enum: uint64_t {
#define NODE_SUBTYPE(subtype) NodeSubtypeLiteral_ ## subtype
	NODE_SUBTYPE(NO) = 0,
	NODE_SUBTYPE(NUMBER) = TokenSubtype_LITERAL_NUMBER,
	NODE_SUBTYPE(CHARACTER) = TokenSubtype_LITERAL_CHARACTER,
	NODE_SUBTYPE(STRING) = TokenSubtype_LITERAL_STRING,
#undef NODE_SUBTYPE_LITERAL
} NodeSubtypeLiteral;

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

#endif
