#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "lexer_allocator.h"

#define CHUNK 4096

static void create_token_null(Token* token) {
	*token = (Token) {
		.type = TokenType_NO,
		.subtype = TokenSubtype_NO,
		.start = 0,
		.end = 0};
}

void lexer_initialize_allocator(Lexer* lexer) {
	assert(lexer != NULL);

	initialize_memory_area(&lexer->tokens);
}

bool lexer_create_allocator_limit(
size_t limit,
Lexer* lexer) {
	assert(limit != 0);
	assert(lexer != NULL);

	if(create_memory_area(
		limit,
		sizeof(Token),
		&lexer->tokens)
	== false)
		return false;

	create_token_null((Token*) lexer->tokens.base);
	return true;
}

bool lexer_create_allocator_chunk(Lexer* lexer) {
	assert(lexer != NULL);

	return lexer_create_allocator_limit(
		CHUNK,
		lexer);
}

bool lexer_allocator_chunk(
size_t minimum,
Lexer* lexer) {
	assert(lexer != NULL);

	if(lexer->tokens.count <= minimum) {
		if(memory_area_realloc(
			(minimum / CHUNK + 1) * CHUNK,
			&lexer->tokens)
		== false) {
			return false;
		}
	}

	return true;
}

bool lexer_allocator_shrink_append_null(Lexer* lexer) {
	// there is at least the same amount of tokens and characters
	assert(lexer->tokens.count <= (size_t) lexer->source->length);

	const bool error = memory_area_realloc(
		lexer->tokens.count + 1, // null token
		&lexer->tokens);
	// for the parser
	create_token_null((Token*) lexer->tokens.base + lexer->tokens.count - 1);
	return error;
}

void lexer_destroy_allocator(Lexer* lexer) {
	if(lexer == NULL)
		return;

	destroy_memory_area(&lexer->tokens);
}

#undef CHUNK
