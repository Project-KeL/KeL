#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "lexer_allocation.h"

#define CHUNK 4096

static void create_token_null(Token* token) {
	*token = (Token) {
		.type = TokenType_NO,
		.subtype = TokenSubtype_NO,
		.start = 0,
		.end = 0};
}

bool lexer_create_allocator(Lexer* lexer) {
	if(create_memory_area(
		CHUNK,
		sizeof(Token),
		&lexer->tokens)
	== false)
		return false;

	create_token_null((Token*) lexer->tokens.addr);
	return true;
}

bool lexer_allocator(
size_t minimum,
Lexer* lexer) {
	if(lexer->tokens.count <= minimum) {
		if(memory_area_realloc(
			(minimum / CHUNK + 1) * CHUNK,
			&lexer->tokens)
		== false)
			return false;	
	}

	return true;
}

bool lexer_allocator_shrink(Lexer* lexer) {
	const bool error = memory_area_realloc(
		lexer->tokens.count + 1, // null token
		&lexer->tokens);
	create_token_null((Token*) lexer->tokens.addr + lexer->tokens.count - 1);
	return error;
}

void lexer_destroy_allocator(Lexer* lexer) {
	destroy_memory_area(&lexer->tokens);
}

#undef CHUNK
