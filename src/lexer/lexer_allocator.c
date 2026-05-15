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
	return lexer_create_allocator_limit(
		CHUNK,
		lexer);
}

bool lexer_allocator_chunk(
size_t minimum,
Lexer* lexer) {
	if(lexer->tokens.count <= minimum) {
		if(memory_area_realloc(
			(minimum / CHUNK + 1) * CHUNK,
			&lexer->tokens)
		== false) {
			assert(false); // the `lexer_create_allocator_limit` must be used
			return false;
		}
	}

	return true;
}

bool lexer_allocator_shrink(Lexer* lexer) {
	// later: must check if lexer->tokens.count is smaller than lexer->source.length
	// TODO: check `source.c` to verify first and last `\0` (if needed, add it)
	// same in `parser_allocator.c`
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
