#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "key_lock.h"
#include "lexer.h"
#include "source.h"

static bool is_eof(char c) {
	return c == '\0';
}

static bool is_significant(char c) {
	return !(c == ' '
	      || c == '\t'
	      || c == '\n'
	      || c == '\r');
}

static bool is_special_action_symbol(char c) {
	return c == '#'
	    || c == '@';
}

static bool is_separator(char c) {
	return is_special_action_symbol(c)
	    || c == ','
	    || c == ';'
	    || c == '['
	    || c == ']';
}

static bool is_colon(char c) {
	return c == ':';
}

static bool is_special(char c) {
	return is_separator(c) || is_colon(c);
}

static bool exhaust_non_significants(
const char* restrict code,
long int* restrict end) {
	long int previous_end = *end;
	while(!is_significant(code[*end])) ++(*end);

	if(previous_end < *end)
		return false;
	else
		return true;
}

static void last_significant_not_special(
const char* restrict code,
long int* restrict end) {
	while(is_significant(code[*end])
	   && !is_special(code[*end])) ++(*end);
}

static bool get_next_word(
const char* restrict code,
long int* start,
long int* end) {
	exhaust_non_significants(
		code,
		end);
	*start = *end;

	if(is_eof(code[*start]))
		return false;
	else if(is_special(code[*start]))
		++(*end);
	else
		last_significant_not_special(
			code,
			end);

	return true;
}

static bool ignore_special_action_symbol(
const char* restrict code,
long int* restrict start,
long int* restrict end) {
	if(is_special(code[*start])) {
		if(is_special_action_symbol(code[*start]))
			--(*end);
		else
			return false;
	}

	return true;
}

static bool get_lock_after_colon(
const char* restrict code,
long int* lock_start,
long int* lock_end) {
	if(is_significant(code[*lock_start])) {
		// case key:lock, key:# or key:@
		get_next_word(
			code,
			lock_start,
			lock_end);
		return ignore_special_action_symbol(
			code,
			lock_start,
			lock_end);
	} else
		return true;
}

static void special_token(
char c,
long int start,
long int end,
Token* restrict token) {
	uint32_t type;

	switch(c) {
		case '#': type = TokenType_HASH; break;
		case ',': type = TokenType_COMMA; break;
		case ':': type = TokenType_COLON; break;
		case ';': type = TokenType_SEMICOLON; break;
		case '@': type = TokenType_AT; break;
		case '[': type = TokenType_LBRACKET; break;
		case ']': type = TokenType_RBRACKET; break;
	}

	*token = (Token) {
		.type = type,
		.has_value = false,
		.start = start,
		.end = end};
}

bool create_lexer(
const Source* source,
Lexer* restrict lexer) {
	lexer->source = NULL;

	const char* restrict code = source->content;
	long int* restrict const count = &lexer->count;
	long int start = 0;
	long int end = 0;

	while(get_next_word(
		code,
		&start,
		&end)
	== true) {
		if(is_special(code[start])) {
			// OK
		} else if(is_valid_key(
			&code[start],
			end - start)
		== true) {
			if(is_eof(code[end])
			|| !is_colon(code[end])) {
				printf("A key must be followed by a colon: \"");
				printf("%.*s\".\n", end - start, &code[start]);
				return false;
			}

			long int lock_start = end + 1;
			long int lock_end = end + 1;

			if(get_lock_after_colon(
				code,
				&lock_start,
				&lock_end)
			== false) {
				printf("A special non special action symbol follows a colon.");
				return false;
			}
			// ignore the case when it is a special action symbol
			if(!is_special_action_symbol(code[end + 1])
			&& is_the_right_lock(
				&code[lock_start],
				lock_end - lock_start)
			== false) {
				printf("Invalid lock: \"%.*s:", end - start, &code[start]);
				printf("%.*s\".\n", lock_end - lock_start, &code[lock_start]);
				return false;
			}

			end = lock_end;
		} else {
			printf("Invalid token \"");
			printf("%.*s\". %d\n", end - start, &code[start], code[start]);
			return false;
		}

		start = end;
		++(*count);
	}

	lexer->tokens = calloc(*count, sizeof(Token));

	if(lexer->tokens == NULL) {
		printf("Allocation error.\n");
		return false;
	}

	lexer->source = source;

	start = 0;
	end = 0;
	long int i = 0;

	while(get_next_word(
		code,
		&start,
		&end)
	== true) {
		if(is_special(code[start])) {	
			special_token(
				code[start],
				start,
				end,
				&lexer->tokens[i]);
		} else {
			long int lock_start = end + 1;
			long int lock_end = end + 1;
			get_lock_after_colon(
				code,
				&lock_start,
				&lock_end);

			if(strncmp(
				&code[start],
				"mov",
				end - start)
			== 0) {
				lexer->tokens[i] = (Token) {
					.type = TokenType_MOV,
					.has_value = false,
					.key_start = start,
					.key_end = end,
					.lock_start = lock_start,
					.lock_end = lock_end};
			} else if(strncmp(
				&code[start],
				"syscall",
				end - start)
			== 0) {
				lexer->tokens[i] = (Token) {
					.type = TokenType_SYSCALL,
					.has_value = false,
					.key_start = start,
					.key_end = end,
					.lock_start = lock_start,
					.lock_end = lock_end};
			}

			end = lock_end;
		}
NEXT:
		start = end;
		++i;
	}

	printf("%ld\n", *count);

	for(long int i = 0; i < *count; ++i)
		printf(
			"%d : %.*s (%.*s)\n",
			lexer->tokens[i].end - lexer->tokens[i].start,
			lexer->tokens[i].end - lexer->tokens[i].start,
			&code[lexer->tokens[i].start],
			lexer->tokens[i].lock_end - lexer->tokens[i].lock_start,
			&code[lexer->tokens[i].lock_start]);

	return true;
}

void destroy_lexer(
Lexer* restrict lexer) {
	lexer->source = NULL;
	free(lexer->tokens);
	lexer->count = 0;
}
