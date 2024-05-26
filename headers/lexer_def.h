#ifndef LEXER_DEF
#define LEXER_DEF

#include <stdint.h>
#include "source.h"

typedef enum: uint32_t {
#define TOKEN_TYPE(type) TokenType_ ## type
	TOKEN_TYPE(NO) = 0,
	TOKEN_TYPE(COLON_LONELY),
	TOKEN_TYPE(COMMAND),
	TOKEN_TYPE(SPECIAL),
	TOKEN_TYPE(QL),
	TOKEN_TYPE(QR),
	TOKEN_TYPE(QLR),
	TOKEN_TYPE(L),
	TOKEN_TYPE(R),
	TOKEN_TYPE(LR),
	TOKEN_TYPE(PL),
	TOKEN_TYPE(IDENTIFIER),
	TOKEN_TYPE(LITERAL),
#undef TOKEN_TYPE
} TokenType;

// in the future, qualifiers will not figure in the subtype (custom qualifiers?)
#define SHIFT_TOKEN_SUBTYPE_QL 8
#define SHIFT_TOKEN_SUBTYPE_QR 16

#define MASK_TOKEN_SUBTYPE_QL 0x0F000000
#define MASK_TOKEN_SUBTYPE_QR 0xF0000000

typedef enum: uint32_t {
#define TOKEN_SUBTYPE(subtype) TokenSubtype_ ## subtype
	TOKEN_SUBTYPE(NO) = 0,
	// litteral
	TOKEN_SUBTYPE(LITERAL_NUMBER), // the base is the highest byte
	TOKEN_SUBTYPE(LITERAL_CHARACTER),
	TOKEN_SUBTYPE(LITERAL_STRING),
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
#undef TOKEN_SUBTYPE
} TokenSubtype;

/*
 * the piece of code is from:
 * - start to R_end for L, R and LR
 * - start to end for the rest
*/

typedef struct {
	uint32_t type;
	uint32_t subtype;
	union {
		struct {
			long int start;
			long int end;};
		struct {
			long int L_start;
			long int L_end;
			long int R_start;
			long int R_end;};};
} Token;

typedef struct {
	const Source* source;
	Token* tokens;
	long int count;
} Lexer;

#endif
