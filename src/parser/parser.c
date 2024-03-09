#include <assert.h>
#include <string.h>
#include "parser.h"
#include "parser_utils.h"
#include <stdio.h>

// later: check errors + use a goto replacing NULL by parser (avoid two almost identical loops)?

bool create_parser(
Lexer* lexer,
Parser* restrict parser) {
#define CMP(str) strncmp( \
	str, \
	&code[token->start], \
	token->end - token->start) \
== 0
	assert(lexer->tokens != NULL);
	assert(lexer->tokens[lexer->count - 1].type == TokenType_NO);
	parser->lexer = lexer;
	parser->nodes = NULL;
	parser->count = 0;

	const char* code = lexer->source->content;
	const Token* tokens = lexer->tokens;
	long int* count = &parser->count;
	long int j = 0;
	
	for(
	long int i = 0;
	i < lexer->count - 1;
	i += 1) {
		switch(tokens[i].type) {
		case TokenType_SPECIAL:
			if(is_at(
				&i,
				j,
				parser)
			== true) {
				// OK
			} else
				return false;

			break;
		default: return false;
		}

		*count += 1;
	}

	*count += 1; // null node
	parser->nodes = calloc(*count, sizeof(Node));

	if(parser->nodes == NULL) {
		printf("Allocation error.\n");
		parser->lexer = NULL;
		return false;
	}
	
	j = 0;

	for(
	long int i = 0;
	i < lexer->count - 1;
	i += 1) {
		switch(tokens[i].type) {
		case TokenType_SPECIAL:
			if(is_at(
				&i,
				j,
				parser)
			== true) {
				// OK
			}

			break;
		default: break;
		}

		j += 1;
	}

	return true;
#undef CMP
}

void destroy_parser(Parser* restrict parser) {
	parser->lexer = NULL;
	free(parser->nodes);
	parser->nodes = NULL;
	parser->count = 0;
}
