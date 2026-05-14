#ifndef NDEBUG
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include "debug_lexer.h"
#include "lexer.h"

static void print_info_token(
const char* code,
const Token* token) {
	const char* type;

	switch(token->type) {
	case TokenType_LSPE: type = "LSPE  "; break;
	case TokenType_RSPE: type = "RSPE  "; break;
	case TokenType_COM: type = "COM   "; break;
	case TokenType_ID: type = "ID    "; break;
	case TokenType_Q: type = "Q     "; break;
	case TokenType_LR: type = "LR    "; break;
	case TokenType_R: type = "R     "; break;
	case TokenType_RSCOPE: type = "RSCOPE"; break;
	case TokenType_L: type = "L     "; break;
	case TokenType_LSCOPE: type = "LSCOPE"; break;
	case TokenType_PL: type = "PL    "; break;
	case TokenType_LIT: type = "LIT   "; break;
	case TokenType_COLON_LONELY: type = "COL   "; break;
	}

	printf(
		"%s\t",
		type);

	switch(token->subtype) {
	case TokenSubtype_SCOPE: printf("SCOPE"); break;
	case TokenSubtype_THEN: printf("THEN"); break;
	case TokenSubtype_MODULE_INPUT: printf("IMOD"); break;
	case TokenSubtype_MODULE_OUTPUT: printf("OMOD"); break;
	default:
		printf(
			"%.*s",
			(int)(token->end - token->start),
			code + token->start);
	}

	switch(token->subtype) {
		case TokenSubtype_LIT_NUM: printf(" (NUM)"); break;
		case TokenSubtype_LIT_CHAR: printf(" (CHAR)"); break;
		case TokenSubtype_LIT_STR: printf(" (STR)"); break;
	}

	printf("\n");
}

void debug_lexer_print_tokens(const Lexer* lexer) {
	printf("TOKENS:\n");

	for(size_t i = 1;
	i < lexer->tokens.count - 1;
	i += 1) {
		printf("\t");
		print_info_token(
			lexer->source->content,
			(Token*) lexer->tokens.addr + i);
	}

	printf(
		"\nNumber of tokens: %zu.\n",
		lexer->tokens.count); 
}

#endif
