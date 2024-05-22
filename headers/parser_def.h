#ifndef PARSER_DEF
#define PARSER_DEF

#include <stdint.h>
#include "lexer_def.h"

typedef enum: uint64_t {
#define NODE_TYPE(type) NodeType_ ## type
	NODE_TYPE(NO) = 0,
	NODE_TYPE(CHILD),
	NODE_TYPE(SCOPE_START), // `.child` holds the ending scope node
	NODE_TYPE(SCOPE_END),
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

/*
 * SCOPE
*/

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

/*
 * IDENTIFICATION
*/

// These are flags (the full token subtype is useless so we extract only the qualifier part)
// The shift let the first bit empty to fetch the command
#define MASK_NODE_SUBTYPE_KEY_IDENTIFICATION_BIT_COMMAND 0b1

typedef enum: uint64_t {
#define NODE_SUBTYPE_KEY_IDENTIFICATION_BIT_COMMAND(subtype) NodeSubtypeKeyIdentificationBitCommand_ ## subtype
	NODE_SUBTYPE_KEY_IDENTIFICATION_BIT_COMMAND(HASH) = 0b0,
	NODE_SUBTYPE_KEY_IDENTIFICATION_BIT_COMMAND(AT) = 0b1,
#undef NODE_SUBTYPE_KEY_IDENTIFICATION_BIT_COMMAND
} NodeSubtypeKeyIdentificationBitCommand;

#define SHIFT_NODE_SUBTYPE_KEY_IDENTIFICATION_BIT_TYPE 1
#define MASK_NODE_SUBTYPE_KEY_IDENTIFICATION_BIT_TYPE 0b10

typedef enum: uint64_t {
#define NODE_SUBTYPE_KEY_IDENTIFICATION_BIT_TYPE(subtype) NodeSubtypeKeyIdentificationBitType_ ## subtype
	NODE_SUBTYPE_KEY_IDENTIFICATION_BIT_TYPE(DECLARATION) = 0b0 << SHIFT_NODE_SUBTYPE_KEY_IDENTIFICATION_BIT_TYPE,
	NODE_SUBTYPE_KEY_IDENTIFICATION_BIT_TYPE(INITIALIZATION) = 0b1 << SHIFT_NODE_SUBTYPE_KEY_IDENTIFICATION_BIT_TYPE,
#undef NODE_SUBTYPE_KEY_IDENTIFICATION_BIT_TYPE
} NodeSubtypeKeyIdentificationBitType;

typedef enum: uint64_t {
#define NODE_SUBTYPE_KEY_QUALIFICATION(subtype) NodeSubtypeKeyQualification_ ## subtype
// WARNING: The following macro is also defined as a static function in "parser_identifier.h"
#define token_subtype_QL_to_subtype(subtype_token) ((subtype_token & MASK_TOKEN_SUBTYPE_QL) >> (SHIFT_TOKEN_SUBTYPE_QL - 2))
	NODE_SUBTYPE_KEY_QUALIFICATION(NO) = token_subtype_QL_to_subtype(TokenSubtype_QL_NO),
	NODE_SUBTYPE_KEY_QUALIFICATION(ENTRY) = token_subtype_QL_to_subtype(TokenSubtype_QL_ENTRY),
	NODE_SUBTYPE_KEY_QUALIFICATION(INC) = token_subtype_QL_to_subtype(TokenSubtype_QL_INC),
	NODE_SUBTYPE_KEY_QUALIFICATION(MUT) = token_subtype_QL_to_subtype(TokenSubtype_QL_MUT),
#undef token_subtype_QL_to_subtype
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

typedef enum: uint64_t {
#define NODE_SUBTYPE_LITERAL(subtype) NodeSubtypeLiteral_ ## subtype
	NODE_SUBTYPE_LITERAL(NO) = 0,
	NODE_SUBTYPE_LITERAL(ASCII) = TokenSubtype_LITERAL_ASCII,
	NODE_SUBTYPE_LITERAL(NUMBER) = TokenSubtype_LITERAL_NUMBER,
	NODE_SUBTYPE_LITERAL(STRING) = TokenSubtype_LITERAL_STRING,
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
