#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>
#include "source.h"

typedef enum: uint32_t {
#define TOKEN_TYPE(type) TokenType_ ## type
	TOKEN_TYPE(NO),
	// separator
	TOKEN_TYPE(HASH),
	TOKEN_TYPE(COMMA),
	TOKEN_TYPE(SEMICOLON),
	TOKEN_TYPE(AT),
	TOKEN_TYPE(LBRACKET),
	TOKEN_TYPE(RBRACKET),
	// colon
	TOKEN_TYPE(COLON),
	// directive
	// key
	TOKEN_TYPE(BYTE),
	TOKEN_TYPE(MOV),
	TOKEN_TYPE(SYSCALL),
#undef TOKEN_TYPE
} TokenType;

typedef struct {
	uint32_t type;
	bool has_value;
	union {
		struct {
			long int start;
			long int end; };
		struct {
			long int key_start;
			long int key_end;
			long int lock_start;
			long int lock_end; };
	};
} Token;

typedef struct {
	const Source* source;
	Token* tokens;
	long int count;
} Lexer;

bool create_lexer(
	const Source* source,
	Lexer* restrict lexer);
void destroy_lexer(
	Lexer* restrict lexer);

#endif
