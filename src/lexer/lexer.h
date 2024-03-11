#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>
#include "../allocator.h"
#include "../source.h"

typedef enum: uint32_t {
#define TOKEN_TYPE(type) TokenType_ ## type
	TOKEN_TYPE(NO) = 0,
	TOKEN_TYPE(SPECIAL),
	TOKEN_TYPE(QUALIFIER_KEY),
	TOKEN_TYPE(QUALIFIER_LOCK),
	TOKEN_TYPE(QUALIFIER_KEY_LOCK),
	TOKEN_TYPE(KEY),
	TOKEN_TYPE(LOCK),
	TOKEN_TYPE(KEYWORD),
	TOKEN_TYPE(IDENTIFIER),
	TOKEN_TYPE(LITERAL),
#undef TOKEN_TYPE
} TokenType;

typedef enum: uint32_t {
#define TOKEN_SUBTYPE(subtype) TokenSubtype_ ## subtype
	TOKEN_SUBTYPE(NO) = 0,
	// litteral
	TOKEN_SUBTYPE(LITERAL_NUMBER),
	TOKEN_SUBTYPE(LITERAL_STRING),
	TOKEN_SUBTYPE(LITERAL_ASCII),
	// special
	TOKEN_SUBTYPE(EXCLAMATION_MARK),
	TOKEN_SUBTYPE(DQUOTES),
	TOKEN_SUBTYPE(HASH),
	TOKEN_SUBTYPE(MODULO),
	TOKEN_SUBTYPE(AMPERSAND),
	TOKEN_SUBTYPE(SQUOTE),
	TOKEN_SUBTYPE(LPARENTHESIS),
	TOKEN_SUBTYPE(RPARENTHESIS),
	TOKEN_SUBTYPE(ASTERISK),
	TOKEN_SUBTYPE(PLUS),
	TOKEN_SUBTYPE(COMMA),
	TOKEN_SUBTYPE(MINUS),
	TOKEN_SUBTYPE(PERIOD),
	TOKEN_SUBTYPE(DIVIDE),
	TOKEN_SUBTYPE(COLON),
	TOKEN_SUBTYPE(SEMICOLON),
	TOKEN_SUBTYPE(LOBRACKET),
	TOKEN_SUBTYPE(EQUAL),
	TOKEN_SUBTYPE(ROBRACKET),
	TOKEN_SUBTYPE(QUESTION_MARK),
	TOKEN_SUBTYPE(AT),
	TOKEN_SUBTYPE(LBRACKET),
	TOKEN_SUBTYPE(RBRACKET),
	TOKEN_SUBTYPE(BACKSLASH),
	TOKEN_SUBTYPE(CARET),
	TOKEN_SUBTYPE(GRAVE_ACCENT),
	TOKEN_SUBTYPE(LCBRACE),
	TOKEN_SUBTYPE(RCBRACE),
	TOKEN_SUBTYPE(PIPE),
	TOKEN_SUBTYPE(TILDE),
	// key qualifier
	TOKEN_SUBTYPE(QUALIFIER_KEY_ENTRY),
	TOKEN_SUBTYPE(QUALIFIER_KEY_INC),
	TOKEN_SUBTYPE(QUALIFIER_KEY_MUT),
	// lock qualifier
	TOKEN_SUBTYPE(QUALIFIER_LOCK_DEFAULT),
	TOKEN_SUBTYPE(QUALIFIER_LOCK_INC),
#undef TOKEN_SUBTYPE
} TokenSubtype;

/*
 * the piece of code is from:
 * - start to lock_end for keys, locks and keywords
 * - start to end for the rest
*/

typedef struct {
	uint32_t type;
	uint32_t subtype1;
	uint32_t subtype2;
	union {
		struct {
			long int start;
			long int end;};
		struct {
			long int key_start;
			long int key_end;
			long int lock_start;
			long int lock_end;};
	};
} Token;

typedef struct {
	const Source* source;
	Token* tokens;
	long int count;
} Lexer;

bool create_lexer(
	const Source* source,
	Allocator* restrict allocator,
	Lexer* restrict lexer);
void destroy_lexer(
	Lexer* restrict lexer);

#endif
