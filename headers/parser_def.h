#ifndef PARSER_DEF
#define PARSER_DEF

#include <stdint.h>
#include "allocator.h"
#include "lexer_def.h"

typedef enum: uint64_t {
#define NODE_TYPE(type) NodeType_ ## type
	NODE_TYPE(NO) = 0,
	NODE_TYPE(MODULE),
	NODE_TYPE(SCOPE_START), // `.child` holds the ending scope node
	NODE_TYPE(SCOPE_END),
	NODE_TYPE(QUALIFIER),
	NODE_TYPE(INTRODUCTION), // `.subtype` holds the command, the type of identifier and the qualifiers
	NODE_TYPE(CALL),
	NODE_TYPE(LITERAL),
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
#undef NODE_TYPE_CHILD
} NodeTypeChild;

typedef enum: uint64_t {
#define NODE_SUBTYPE_CHILD(subtype) NodeSubtypeChild_ ## subtype
	NODE_SUBTYPE_CHILD(NO) = 0,
#undef NODE_SUBTYPE_CHILD
} NodeSubtypeChild;

/*
 * MODULE
*/

typedef enum: uint64_t {
#define NODE_SUBTYPE(subtype) NodeSubtypeModule_ ## subtype
	NODE_SUBTYPE(NO) = 0,
	NODE_SUBTYPE(INPUT),
	NODE_SUBTYPE(OUTPUT),
#undef NODE_SUBTYPE
} NodeSubtypeModule;

/*
 * TYPE
*/

typedef enum: uint64_t {
#define NODE_TYPE_CHILD(type) NodeTypeChildType_ ## type
	NODE_TYPE_CHILD(NO) = 0,
	NODE_TYPE_CHILD(MODIFIER),
	NODE_TYPE_CHILD(LOCK),
#undef NODE_TYPE_CHILD
} NodeTypeChildType;

typedef enum: uint64_t {
#define NODE_SUBTYPE_CHILD(subtype) NodeSubtypeChildType_ ## subtype
	NODE_SUBTYPE_CHILD(NO),
#undef NODE_SUBTYPE_CHILD
} NodeSubtypeChildType;

typedef enum: uint64_t {
#define NODE_SUBTYPE_CHILD(subtype) NodeSubtypeChildTypeScoped_ ## subtype
	NODE_SUBTYPE_CHILD(RETURN_NONE) = 1,
	NODE_SUBTYPE_CHILD(RETURN_TYPE),
	NODE_SUBTYPE_CHILD(PARAMETER_NONE),
	NODE_SUBTYPE_CHILD(PARAMETER),
#undef NODE_SUBTYPE_CHILD
} NodeSubtypeChildTypeScoped;

/*
 * SCOPE
*/

typedef enum: uint64_t {
#define NODE_SUBTYPE(subtype) NodeSubtypeScopeStart_ ## subtype
	NODE_SUBTYPE(NO) = 0,
	NODE_SUBTYPE(THEN),
	NODE_SUBTYPE(THEN_NOT),
	NODE_SUBTYPE(THROUGH),
	NODE_SUBTYPE(THROUGH_NOT),
	NODE_SUBTYPE(TEST),
#undef NODE_SUBTYPE
} NodeSubtypeScopeStart;

typedef enum: uint64_t {
#define NODE_SUBTYPE(subtype) NodeSubtypeScopeEnd_ ## subtype
	NODE_SUBTYPE(NO),
#undef NODE_SUBTYPE
} NodeSubtypeScopeEnd;

/*
 * IDENTIFICATION
*/

#define MASK_BIT_NODE_SUBTYPE_INTRODUCTION_COMMAND 0b11

typedef enum: uint64_t {
#define NODE_SUBTYPE_BIT(subtype) NodeSubtypeIntroductionBitCommand_ ## subtype
	NODE_SUBTYPE_BIT(HASH) = 0b00,
	NODE_SUBTYPE_BIT(AT) = 0b01,
	NODE_SUBTYPE_BIT(EXCLAMATION_MARK) = 0b10,
#undef NODE_SUBTYPE_BIT
} NodeSubtypeIntroductionBitCommand;

#define SHIFT_BIT_NODE_SUBTYPE_INTRODUCTION_TYPE 2
#define MASK_BIT_NODE_SUBTYPE_INTRODUCTION_TYPE (0b1 << SHIFT_BIT_NODE_SUBTYPE_INTRODUCTION_TYPE)

typedef enum: uint64_t {
#define NODE_SUBTYPE_BIT(subtype) NodeSubtypeIntroductionBitType_ ## subtype
	NODE_SUBTYPE_BIT(DECLARATION) = 0b0 << SHIFT_BIT_NODE_SUBTYPE_INTRODUCTION_TYPE,
	NODE_SUBTYPE_BIT(INITIALIZATION) = 0b1 << SHIFT_BIT_NODE_SUBTYPE_INTRODUCTION_TYPE,
#undef NODE_SUBTYPE_BIT
} NodeSubtypeIntroductionBitType;

#define SHIFT_BIT_NODE_SUBTYPE_INTRODUCTION_SCOPED 3
#define MASK_BIT_NODE_SUBTYPE_INTRODUCTION_SCOPED (0b11 << SHIFT_BIT_NODE_SUBTYPE_INTRODUCTION_SCOPED)

typedef enum: uint64_t {
#define NODE_SUBTYPE_BIT(subtype) NodeSubtypeIntroductionBitScoped_ ## subtype
	NODE_SUBTYPE_BIT(NO) = 0b00 << SHIFT_BIT_NODE_SUBTYPE_INTRODUCTION_SCOPED,
	NODE_SUBTYPE_BIT(LABEL) = 0b01 << SHIFT_BIT_NODE_SUBTYPE_INTRODUCTION_SCOPED,
	NODE_SUBTYPE_BIT(PAL) = 0b10 << SHIFT_BIT_NODE_SUBTYPE_INTRODUCTION_SCOPED,
#undef NODE_SUBTYPE_BIT
} NodeSubtypeIntroductionBitScoped;

/*
 * CALLS
*/

#define MASK_BIT_NODE_SUBTYPE_CALL_TIME 0b11

typedef enum: uint64_t {
#define NODE_SUBTYPE_BIT(subtype) NodeSubtypeCallBitTime_ ## subtype
	NODE_SUBTYPE_BIT(COMPILE) = 0b00,
	NODE_SUBTYPE_BIT(BINARY) = 0b01,
	NODE_SUBTYPE_BIT(RUN) = 0b10,
#undef NODE_SUBTYPE_BIT
} NodeSubtypeCallBitTime;

#define SHIFT_BIT_NODE_SUBTYPE_CALL_RETURN 2
#define MASK_BIT_NODE_SUBTYPE_CALL_RETURN (0b1 << SHIFT_BIT_NODE_SUBTYPE_CALL_RETURN)

typedef enum: uint64_t {
#define NODE_SUBTYPE_BIT(subtype) NodeSubtypeCallBitReturn_ ## subtype
	NODE_SUBTYPE_BIT(FALSE) = 0b0 << SHIFT_BIT_NODE_SUBTYPE_CALL_RETURN,
	NODE_SUBTYPE_BIT(TRUE) = 0b1 << SHIFT_BIT_NODE_SUBTYPE_CALL_RETURN,
#undef NODE_SUBTYPE_BIT
} NodeSubtypeCallBitReturn; // the type of the returned value is explicit?

#define SHIFT_BIT_NODE_SUBTYPE_CALL_RETURN_DEDUCE 3
#define MASK_BIT_NODE_SUBTYPE_CALL_RETURN_DEDUCE (0b1 << SHIFT_BIT_NODE_SUBTYPE_CALL_RETURN_DEDUCE)

typedef enum: uint64_t {
#define NODE_SUBTYPE_BIT(subtype) NodeSubtypeCallBitReturnDeduce_ ## subtype
	NODE_SUBTYPE_BIT(FALSE) = 0b0 << SHIFT_BIT_NODE_SUBTYPE_CALL_RETURN_DEDUCE,
	NODE_SUBTYPE_BIT(TRUE) = 0b1 << SHIFT_BIT_NODE_SUBTYPE_CALL_RETURN_DEDUCE,
#undef NODE_SUBTYPE_BIT
} NodeSubtypeCallBitReturnDeduce; // the true case will be used during parsing expressions only

typedef enum: uint64_t {
#define NODE_TYPE_CHILD(type) NodeTypeChildCall_ ## type
	NODE_TYPE_CHILD(NO) = 0,
	NODE_TYPE_CHILD(RETURN_UNKNOWN), // may return a type to be deduced or no return
	NODE_TYPE_CHILD(RETURN_TYPE),
	NODE_TYPE_CHILD(ARGUMENT_NONE),
	NODE_TYPE_CHILD(ARGUMENT),
#undef NODE_TYPE_CHILD
} NodeTypeChildCall;

typedef enum: uint64_t {
#define NODE_SUBTYPE_CHILD(subtype) NodeSubtypeChildCall_ ## subtype
	NODE_SUBTYPE_CHILD(NO) = 0,
#undef NODE_SUBTYPE_CHILD
} NodeSubtypeChildCall;

/*
 * MODIFIERS (to be done)
*/

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
	bool is_child;
	uint64_t type;
	uint64_t subtype;
	union {
		uint64_t value;
		void* value_ptr;
		void (*value_fn)();
		const Token* token;};
	union {
		Node* child;
		struct NodeTreeModule {Node* next;} Module;
		struct NodeTreeType {Node* next;} Type;
		struct NodeTreeScopeStart {
			Node* scope_end;
			Node* PAL;} ScopeStart;
		struct NodeTreeScopeEnd {Node* scope_start;} ScopeEnd;
		struct NodeTreeQualifier {Node* next;} Qualifier;
		struct NodeTreeIdentifier {
			Node* type;
			Node* initialization;
			Node* qualifiers;} Introduction;
		struct NodeTreeCall {
			Node* PAL;
			Node* arguments;} Call;
		struct NodeTreeChildCall {Node* next;} ChildCall;
	};
};

typedef struct {
	const Lexer* lexer;
	MemoryChain nodes;
	MemoryChain file_nodes; // declarations at file scope
} Parser;

#endif
