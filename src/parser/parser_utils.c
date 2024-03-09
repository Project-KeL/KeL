#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "../lexer/lexer_utils.h"
#include "parser_utils.h"

#define CMP_LOCK(str) strncmp( \
	str, \
	&code[tokens[buffer_i].lock_start], \
	tokens[buffer_i].lock_end - tokens[buffer_i].lock_start) == 0

static uint64_t string_to_uint64(
const char* code,
long int start,
long int end) {
	uint_fast8_t base = 10;

	if(code[start] == '0'
	&& !is_digit(code[start + 1])) {
		switch(code[start + 1]) {
		case 'b': base = 2; break;
		case 'o': base = 8; break;
		case 'x': base = 16; break;
		default: assert(false);
		}

		start += 2;
	}
	// ignore zeros
	while(code[start] == '0') start += 1;
	uint64_t ret = 0;

	for(long int i = start;
	i < end;
	i += 1) {
		printf("%d, %c\n", end - start, code[i]); 
		assert(is_digit(code[i])
			|| (code[i] > 64
			 && code[i] < 71));
		assert((base < 9
			 && code[i] - '0' < base)
			|| (base == 16
			 && (code[i] > 64
		      || code[i] < 71)));

		if(code[i] > 64
		&& code[i] < 71) {
			assert(base == 16);
			ret = 16 * ret + 10 + code[i] - 'A';
		} else
			ret = base * ret + code[i] - '0';
	}

	return ret;
}

static bool is_core_feature(
long int* i,
long int j,
Parser* parser) {
	const char* code = parser->lexer->source->content;
	const Token* tokens = parser->lexer->tokens;
	long int buffer_i = *i;
	Node buffer_node;

	if(CMP_LOCK("byte")) {
		buffer_i += 1;

		if(tokens[buffer_i].subtype1 != TokenSubtype_LCBRACE)
			return false;

		buffer_i += 1;

		if(tokens[buffer_i].type != TokenType_LITERAL
		&& (tokens[buffer_i].subtype1 != TokenSubtype_LITERAL_NUMBER
		 || tokens[buffer_i].subtype1 != TokenSubtype_LITERAL_ASCII))
			return false;

		if(parser->nodes != NULL) {
			buffer_node = (Node) {
				.type = NodeType_CORE_B,
				.value = string_to_uint64( // check errors (between 0x00 and 0xFF)
					code,
					tokens[buffer_i].key_start,
					tokens[buffer_i].key_end),
				.child1 = NULL,
				.child2 = NULL};
		}

		buffer_i += 1;

		if(tokens[buffer_i].subtype1 != TokenSubtype_RCBRACE)
			return false; // syntax error!

		buffer_i += 1;

		if(tokens[buffer_i].subtype1 != TokenSubtype_SEMICOLON)
			return false; // syntax error!
	} else
		return false;

	*i = buffer_i;

	if(parser->nodes != NULL)
		parser->nodes[j] = buffer_node;

	return true;
}

bool is_at(
long int* i,
long int j,
Parser* parser) {
	assert(parser->lexer != NULL);
	long int buffer_i = *i + 1;

	// key case
	
	// lock case
	if(is_core_feature(
		&buffer_i,
		j,
		parser)
	== true) {
		// OK
	} else
		return false;	

	*i = buffer_i;
	return true;
}

#undef CMP_LOCK
