#ifndef PARSER_DEF
#define PARSER_DEF

#include <stdint.h>
#include "lexer_def.h"

typedef enum: uint64_t {
#define NODE_TYPE(type) NodeType_ ## type
	NODE_TYPE(NO) = 0,
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

typedef enum: uint64_t {
	NodeTypeChild_NO = 0,
} NodeTypeChild;

typedef enum: uint64_t {
	NodeSubtypeChild_NO = 0,
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
#define MASK_BIT_NODE_SUBTYPE_KEY_IDENTIFICATION_COMMAND 0b1

typedef enum: uint64_t {
#define NODE_SUBTYPE_KEY_IDENTIFICATION(subtype) NodeSubtypeKeyIdentificationBitCommand_ ## subtype
	NODE_SUBTYPE_KEY_IDENTIFICATION(HASH) = 0b0,
	NODE_SUBTYPE_KEY_IDENTIFICATION(AT) = 0b1,
#undef NODE_SUBTYPE_KEY_IDENTIFICATION
} NodeSubtypeKeyIdentificationBitCommand;

#define SHIFT_NODE_SUBTYPE_KEY_IDENTIFICATION_BIT_TYPE 1
#define MASK_BIT_NODE_SUBTYPE_KEY_IDENTIFICATION_TYPE (1 << SHIFT_NODE_SUBTYPE_KEY_IDENTIFICATION_BIT_TYPE)

typedef enum: uint64_t {
#define NODE_SUBTYPE_KEY_IDENTIFICATION(subtype) NodeSubtypeKeyIdentificationBitType_ ## subtype
	NODE_SUBTYPE_KEY_IDENTIFICATION(DECLARATION) = 0b0 << SHIFT_NODE_SUBTYPE_KEY_IDENTIFICATION_BIT_TYPE,
	NODE_SUBTYPE_KEY_IDENTIFICATION(INITIALIZATION) = 0b1 << SHIFT_NODE_SUBTYPE_KEY_IDENTIFICATION_BIT_TYPE,
#undef NODE_SUBTYPE_KEY_IDENTIFICATION
} NodeSubtypeKeyIdentificationBitType;

#define SHIFT_NODE_SUBTYPE_KEY_IDENTIFICATION_BIT_SCOPE 2
#define MASK_BIT_NODE_SUBTYPE_KEY_IDENTIFICATION_SCOPE (1 << SHIFT_NODE_SUBTYPE_KEY_IDENTIFICATION_BIT_SCOPE)

typedef enum: uint64_t {
#define NODE_SUBTYPE_KEY_IDENTIFICATION(subtype) NodeSubtypeKeyIdentificationBitScoped_ ## subtype
	NODE_SUBTYPE_KEY_IDENTIFICATION(FALSE) = 0,
	NODE_SUBTYPE_KEY_IDENTIFICATION(TRUE) = 1,
#undef NODE_SUBTYPE_KEY_IDENTIFICATION
} NodeSubtypeKeyIdentificationBitScoped;

typedef enum: uint64_t {
#define NODE_SUBTYPE_KEY(subtype) NodeSubtypeKeyQualification_ ## subtype
// WARNING: The following macro is also defined as a static function in "parser_identifier.h" but all this mess will be removed later
#define token_subtype_QL_to_subtype(subtype_token) ((subtype_token & MASK_TOKEN_SUBTYPE_QL) >> (SHIFT_TOKEN_SUBTYPE_QL - 3))
	NODE_SUBTYPE_KEY(NO) = token_subtype_QL_to_subtype(TokenSubtype_QL_NO),
	NODE_SUBTYPE_KEY(ENTRY) = token_subtype_QL_to_subtype(TokenSubtype_QL_ENTRY),
	NODE_SUBTYPE_KEY(INC) = token_subtype_QL_to_subtype(TokenSubtype_QL_INC),
	NODE_SUBTYPE_KEY(MUT) = token_subtype_QL_to_subtype(TokenSubtype_QL_MUT),
#undef token_subtype_QL_to_subtype
#undef NODE_SUBTYPE_KEY
} NodeSubtypeKeyQualification;

/*
 * `LEFT` and `RIGHT` is relative to the lock
 * if an array has a bound, it is set in `.value`
*/

typedef enum: uint64_t {
#define NODE_TYPE_CHILD_KEY_TYPE(type) NodeTypeChildKeyType_ ## type
	NODE_TYPE_CHILD_KEY_TYPE(NO) = 0,
	NODE_TYPE_CHILD_KEY_TYPE(MODIFIER),
	NODE_TYPE_CHILD_KEY_TYPE(LOCK),
#undef NODE_TYPE_CHILD_KEY_TYPE
} NodeTypeChildKeyType;

typedef enum: uint64_t {
#define NODE_SUBTYPE_CHILD_KEY_TYPE(subtype) NodeSubtypeChildKeyTypeModifier_ ## subtype
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
} NodeSubtypeChildKeyTypeModifier;

typedef enum: uint64_t {
#define NODE_SUBTYPE_CHILD_KEY_TYPE(subtype) NodeSubtypeChildKeyTypeScoped_ ## subtype
	NODE_SUBTYPE_CHILD_KEY_TYPE(RETURN_NONE) = 1,
	NODE_SUBTYPE_CHILD_KEY_TYPE(RETURN_LOCK),
	NODE_SUBTYPE_CHILD_KEY_TYPE(PARAMETER_NONE),
	NODE_SUBTYPE_CHILD_KEY_TYPE(PARAMETER),
	NODE_SUBTYPE_CHILD_KEY_TYPE(PARAMETER_LOCK),
	NODE_SUBTYPE_CHILD_KEY_TYPE(PARAMETER_RETURN_LOCK),
#undef NODE_SUBTYPE_CHILD_KEY_TYPE
} NodeSubtypeChildKeyTypeLock;

/*
 * LITERAL
*/

typedef enum: uint64_t {
#define NODE_SUBTYPE(subtype) NodeSubtypeLiteral_ ## subtype
	NODE_SUBTYPE(NO) = 0,
	NODE_SUBTYPE(ASCII) = TokenSubtype_LITERAL_ASCII,
	NODE_SUBTYPE(NUMBER) = TokenSubtype_LITERAL_NUMBER,
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
