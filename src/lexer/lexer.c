#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "lexer_error.h"
#include "lexer_utils.h"

/*
 * checking order:
 * 1 - is_command (important to check the key case)
 * 2 - is_qualifier_key
 * 3 - is_key
 * 4 - is_qualifier_lock
 * 5 - is_lock
 * 6 - is_qualifier_key_lock
 * 7 - is_keyword
 * 8 - is_delimiter
 * 9 - is_literal
 * 10 - is_valid_name
 *
 * qualifier cases are checked first to detect the brackets, so it is easier to detect
 * names (for instance, the name of a key or a lock).
 */

// more errors will be implemented in the dedicated function
// this variable manages errors which subsequently may no longer be errors
static bool token_error = false;

static bool is_valid_qualifier_key(
const char* code,
long int start,
long int end,
TokenSubtype* subtype) {
	*subtype = TokenSubtype_NO;
	code = &code[start]; // for strcmp

	if(strcmp(
		"entry",
		code)
	< 0)
		*subtype = TokenSubtype_QUALIFIER_KEY_ENTRY;
	else if(strcmp(
		"mut",
		code)
	< 0)
		*subtype = TokenSubtype_QUALIFIER_KEY_MUT;

	return *subtype != TokenSubtype_NO;
}

static bool is_valid_qualifier_lock(
const char* code,
long int start,
long int end,
TokenSubtype* subtype) {
	*subtype = TokenSubtype_NO;
	code = &code[start];

	if(strcmp(
		"default",
		code)
	< 0)
		*subtype = TokenSubtype_QUALIFIER_LOCK_DEFAULT;

	return *subtype != TokenSubtype_NO;
}

static bool is_literal(
const char* restrict code,
long int start,
long int* end,
TokenSubtype* subtype1,
TokenSubtype* subtype2) {
	long int buffer_end = start + 1;

	if(is_digit(code[start])) {
		*subtype2 = 10;

		if(code[start] != '0'
		|| is_digit(code[buffer_end]))
			goto NO_BASE_CHECK;

		switch(code[buffer_end]) {
			case 'b': *subtype2 = 2; break;
			case 'o': *subtype2 = 8; break;
			case 'x': *subtype2 = 16; break;
			default: token_error = true; return false; // unknown base
		}

		buffer_end += 1;

		if(!is_digit(code[buffer_end])
		&& (code[buffer_end] <= 64
		 && code[buffer_end] >= 71)) {
			token_error = true;
			return false;
		}
NO_BASE_CHECK:
		while(!is_eof(code[buffer_end]) 
		   && (is_digit(code[buffer_end])
		    || (code[buffer_end] > 64
		     && code[buffer_end] < 71)
		    || code[buffer_end] == '`')) buffer_end += 1;
		// a number cannot be followed by '`' and must be followed by a blank or a special symbole
		if(code[buffer_end - 1] == '`'
		|| (is_significant(code[buffer_end])
		 && !is_special(code[buffer_end]))) {
			token_error = true;
			return false;
		}

		*subtype1 = TokenSubtype_LITERAL_NUMBER;
	} else if(code[start] == '`') {
		while(!is_eof(code[buffer_end])
		   && code[buffer_end] != '`') buffer_end += 1;

		if(code[buffer_end] != '`') {
			token_error = true;
			return false;
		}

		buffer_end += 1;
		*subtype2 = TokenSubtype_LITERAL_STRING;
	} else if(code[start] == '\\') {
		if(!is_alphabetical(code[buffer_end])
		&& !is_digit(code[buffer_end])) {
			token_error = true;
			return false;
		}

		buffer_end += 1;
		*subtype1 = TokenSubtype_LITERAL_ASCII;
	} else
		return false;

	*end = buffer_end;
	return true;
}

static bool get_qualifier_key(
const char* restrict code,
long int start,
long int* end,
long int* key_start,
long int* key_end,
TokenSubtype* subtype) {
	if(code[start] != '[')
		return false;

	long int buffer_end = start + 1;
	get_next_word(
		code,
		&start,
		&buffer_end);

	if(is_special(code[start])) {
		token_error = true;
		return false;
	}

	if(is_valid_qualifier_key(
		code,
		start,
		buffer_end,
		subtype)
	== false)
		return false;

	*key_start = start;
	*key_end = buffer_end;
	get_next_word(
		code,
		&start,
		&buffer_end);
	// behaviour not set yet (several qualifiers)
	if(code[start] != ']') {
		token_error = true;
		return false;
	}

	if(code[buffer_end] != ':')
		return false;

	*end = buffer_end;
	return true;
}

#include <stdio.h>
static bool is_qualifier_key(
const char* restrict code,
long int start,
long int* end,
long int* key_start,
long int* key_end,
TokenSubtype* subtype) {
	long int buffer_end = *end;

	if(!get_qualifier_key(
		code,
		start,
		&buffer_end,
		key_start,
		key_end,
		subtype)
	|| is_significant(code[buffer_end + 1]))
		return false;

	*end = buffer_end + 1;
	return true;
}

static bool is_key(
const char* restrict code,
long int start,
long int* end,
bool previous_is_command) {
	if(!previous_is_command
	&& code[*end] == ':'
	&& is_valid_name(
		code,
		start,
		*end)
	== true) {
		*end += 1; // skip the ':'
		return true;
	} else
		return false;
}

static bool get_qualifier_lock(
const char* restrict code,
long int start,
long int* end,
long int* lock_start,
long int* lock_end,
TokenSubtype* subtype) {
	if(code[start] != ':'
	|| code[start + 1] != '['
	|| is_significant(code[start - 1]))
		return false;

	start += 1;
	long int buffer_end = start + 1;
	get_next_word(
		code,
		&start,
		&buffer_end);

	if(is_valid_qualifier_lock(
		code,
		start,
		buffer_end,
		subtype)
	== false)
		return false;

	*lock_start = start;
	*lock_end = buffer_end;
	get_next_word(
		code,
		&start,
		&buffer_end);
	// behaviour not set yet (several qualifiers)
	if(code[start] != ']') {
		token_error = true;
		return false;
	}

	*end = buffer_end;
	return true;
}

static bool is_qualifier_lock(
const char* restrict code,
long int start,
long int* end,
long int* lock_start,
long int* lock_end,
TokenSubtype* subtype) {
	long int buffer_end = *end;

	if(!get_qualifier_lock(
		code,
		start,
		&buffer_end,
		lock_start,
		lock_end,
		subtype)
	|| is_significant(code[buffer_end]))
		return false;

	*end = buffer_end;
	return true;
}

static bool is_lock(
const char* restrict code,
long int start,
long int* end) {
	if(code[start] != ':')
		return false;

	long int buffer_end = start + 1;
	get_next_word(
		code,
		&start,
		&buffer_end);
	// a colon is a token only in the pointer case
	if(is_special(code[*end])) {
		token_error = true;
		return false;
	}

	if(is_valid_name(
		code,
		start,
		buffer_end)
	== false)
		return false; // could be an array

	*end = buffer_end;
	return true;
}

static bool is_qualifier_key_lock(
const char* restrict code,
long int start,
long int* end,
long int* key_start,
long int* key_end,
long int* lock_start,
long int* lock_end,
TokenSubtype* subtype1,
TokenSubtype* subtype2) {
	long int buffer_end = *end;
	
	if(get_qualifier_key(
		code,
		start,
		&buffer_end,
		key_start,
		key_end,
		subtype1)
	== false)
		return false;

	start = buffer_end;

	if(get_qualifier_lock(
		code,
		start,
		&buffer_end,
		lock_start,
		lock_end,
		subtype2)
	== false)
		return false;

	if(is_significant(code[buffer_end]))
		return false;

	*end = buffer_end;
	return true;
}

static bool is_keyword(
const char* restrict code,
long int start,
long int end,
long int* lock_start,
long int* lock_end,
bool previous_is_command) {
	if(!previous_is_command
	&& is_valid_name(
		code,
		start,
		end)
	&& code[end] == ':') {
		long int buffer_lock_start = end + 1;
		long int buffer_lock_end = end + 1;
		get_next_word(
			code,
			&buffer_lock_start,
			&buffer_lock_end);

		if(is_valid_name(
			code,
			buffer_lock_start,
			buffer_lock_end)
		== false)
			return false;

		*lock_start = buffer_lock_start;
		*lock_end = buffer_lock_end;
		return true;
	}

	return false;
}

static void create_token_special(
const char* restrict code,
long int start,
long int end,
Token* restrict token) {
	uint32_t subtype;

	switch(code[start]) {
	case '!': subtype = TokenSubtype_EXCLAMATION_MARK; break;
	case '"': subtype = TokenSubtype_DQUOTES; break;
	case '#': subtype = TokenSubtype_HASH; break;
	case '%': subtype = TokenSubtype_MODULO; break;
	case '&': subtype = TokenSubtype_AMPERSAND; break;
	case '\'': subtype = TokenSubtype_SQUOTE; break;
	case '(': subtype = TokenSubtype_LPARENTHESIS; break;
	case ')': subtype = TokenSubtype_RPARENTHESIS; break;
	case '*': subtype = TokenSubtype_ASTERISK; break;
	case '+': subtype = TokenSubtype_PLUS; break;
	case ',': subtype = TokenSubtype_COMMA; break;
	case '-': subtype = TokenSubtype_MINUS; break;
	case '.': subtype = TokenSubtype_PERIOD; break;
	case '/': subtype = TokenSubtype_DIVIDE; break;
	case ':': subtype = TokenSubtype_COLON; break;
	case ';': subtype = TokenSubtype_SEMICOLON; break;
	case '<': subtype = TokenSubtype_LOBRACKET; break;
	case '=': subtype = TokenSubtype_EQUAL; break;
	case '>': subtype = TokenSubtype_ROBRACKET; break;
	case '?': subtype = TokenSubtype_QUESTION_MARK; break;
	case '@': subtype = TokenSubtype_AT; break;
	case '[': subtype = TokenSubtype_LBRACKET; break;
	case ']': subtype = TokenSubtype_RBRACKET; break;
	case '\\': subtype = TokenSubtype_BACKSLASH; break;
	case '^': subtype = TokenSubtype_CARET; break;
	case '`': subtype = TokenSubtype_GRAVE_ACCENT; break;
	case '{': subtype = TokenSubtype_LCBRACE; break;
	case '|': subtype = TokenSubtype_PIPE; break;
	case '}': subtype = TokenSubtype_RCBRACE; break;
	case '~': subtype = TokenSubtype_TILDE; break;
	default: assert(false); // missing case
	}

	*token = (Token) {
		.type = TokenType_SPECIAL,
		.subtype1 = subtype,
		.has_value = false,
		.start = start,
		.end = end};
}

static void create_token_literal(
TokenSubtype subtype1,
TokenSubtype subtype2,
long int start,
long int end,
Token* token) {
	*token = (Token) {
		.type = TokenType_LITERAL,
		.subtype1 = subtype1,
		.subtype2 = subtype2,
		.subtype2 = TokenSubtype_NO,
		.has_value = true,
		.start = start,
		.end = end};
}

static void create_qualifier_token(
TokenType type,
TokenSubtype subtype,
long int start,
long int end,
Token* restrict token) {
	*token = (Token) {
		.type = type,
		.subtype1 = subtype,
		.subtype2 = TokenSubtype_NO,
		.has_value = false,
		.start = start,
		.end = end};
}

static void create_colon_word_token(
TokenType type,
long int key_start,
long int key_end,
long int lock_start,
long int lock_end,
Token* restrict token) {
	*token = (Token) {
		.type = type,
		.subtype1 = TokenSubtype_NO,
		.subtype2 = TokenSubtype_NO,
		.has_value = false,
		.key_start = key_start,
		.key_end = key_end,
		.lock_start = lock_start,
		.lock_end = lock_end};
}

bool create_lexer(
const Source* source,
Allocator* restrict allocator,
Lexer* restrict lexer) {
	lexer->source = NULL;
	lexer->tokens = NULL;
	lexer->count = 0;

	const char* code = source->content;
	long int* const count = &lexer->count;
	bool previous_is_command = false; // checked for keys only
	long int start = 0;
	long int end = 0;
	long int key_start;
	long int key_end;
	long int lock_start;
	long int lock_end;

	if(lexer_scan_errors(
		source,
		allocator)
	== false)
		return false;

	while(get_next_word(
		code,
		&start,
		&end)
	== true) {
		TokenSubtype subtype1;
		TokenSubtype subtype2;

		if(is_command(code[start])) {
			// OK
		} else if(is_qualifier_key(
			code,
			start,
			&end,
			&key_start,
			&key_end,
			&subtype1)
		== true) {
			// OK
		} else if(is_key(
			code,
			start,
			&end,
			previous_is_command)
		== true)  {
			// OK
		} else if(is_qualifier_lock(
			code,
			start,
			&end,
			&lock_start,
			&lock_end,
			&subtype1)
		== true) {
			// OK
		} else if(is_lock(
			code,
			start,
			&end)
		== true) {
			// OK
		} else if(is_qualifier_key_lock(
			code,
			start,
			&end,
			&key_start,
			&key_end,
			&lock_start,
			&lock_end,
			&subtype1,
			&subtype2)
		== true) {
			// OK
		} else if(is_keyword(
			code,
			start,
			end,
			&lock_start,
			&lock_end,
			previous_is_command)
		== true) {
			// OK
			end = lock_end;
		} else if(is_delimiter(code[start])) {
			// OK
		} else if(is_literal(
			code,
			start,
			&end,
			&subtype1,
			&subtype2)
		== true) {
			// OK
		} else if(is_special(code[start])) {
			// OK
		} else if(is_valid_name(
			code,
			start,
			end)
		== true) {
			// OK
		} else {
printf("%.*s\n", end - start, &code[start]);
			return false;
		}

		previous_is_command = is_command(code[start]);
		start = end;
		*count += 1;
	}

	*count += 1; // null token
	lexer->tokens = calloc(*count, sizeof(Token));

	if(lexer->tokens == NULL) {
		printf("Allocation error.\n");
		return false;
	}

	lexer->source = source;

	previous_is_command = false;
	start = 0;
	end = 0;
	long int i = 0;

	while(get_next_word(
		code,
		&start,
		&end)
	== true) {
		assert(i < *count - 1);
		Token* token = &lexer->tokens[i];
		TokenSubtype subtype1;
		TokenSubtype subtype2;

		if(is_command(code[start])) {
			create_token_special(
				code,
				start,
				end,
				token);
			previous_is_command = true;
		} else if(is_qualifier_key(
			code,
			start,
			&end,
			&key_start,
			&key_end,
			&subtype1)
		== true) {
			create_qualifier_token(
				TokenType_QUALIFIER_KEY,
				subtype1,
				key_start,
				key_end,
				token);
		} else if(is_key(
			code,
			start,
			&end,
			previous_is_command)
		== true)  {
			create_colon_word_token(
				TokenType_KEY,
				start,
				end - 1, // ignore colon
				end,
				end,
				token);
		} else if(is_qualifier_lock(
			code,
			start,
			&end,
			&lock_start,
			&lock_end,
			&subtype1)
		== true) {
			create_qualifier_token(
				TokenType_QUALIFIER_LOCK,
				subtype1,
				lock_start,
				lock_end,
				token);
		} else if(is_lock(
			code,
			start,
			&end)
		== true) {
			create_colon_word_token(
				TokenType_LOCK,
				start,
				start,
				start + 1, // ignore colon
				end,
				token);
		} else if(is_qualifier_key_lock(
			code,
			start,
			&end,
			&key_start,
			&key_end,
			&lock_start,
			&lock_end,
			&subtype1,
			&subtype2)
		== true) {
			token->type = TokenType_QUALIFIER_KEY_LOCK;
			token->subtype1 = subtype1;
			token->subtype2 = subtype2;
			token->has_value = false;
			token->key_start = key_start;
			token->key_end = key_end;
			token->lock_start = lock_start;
			token->lock_end = lock_end;
		} else if(is_keyword(
			code,
			start,
			end,
			&lock_start,
			&lock_end,
			previous_is_command)
		== true) {
			create_colon_word_token(
				TokenType_KEYWORD,
				start,
				end,
				lock_start,
				lock_end,
				token);
			end = lock_end;
		} else if(is_delimiter(code[start])) {
			create_token_special(
				code,
				start,
				end,
				token);
		} else if(is_literal(
			code,
			start,
			&end,
			&subtype1,
			&subtype2)
		== true) {
				*token = (Token) {
					.type = TokenType_LITERAL,
					.subtype1 = subtype1,
					.subtype2 = subtype2,
					.has_value = true,
					.start = start,
					.end = end};
		} else if(is_special(code[start])) {
			create_token_special(
				code,
				start,
				end,
				token);
		} else if(is_valid_name(
			code,
			start,
			end)
		== true) {
			*token = (Token) = {
				.type = TokenType_IDENTIFIER,
				.subtype1 = TokenSubtype_NO,
				.subtype2 = TokenSubtype_NO,
				.has_value = false,
				.start = start,
				.end = end};
		} else {
		}

		previous_is_command = is_command(code[start]);
		start = end;
		i += 1;
	}

	lexer->tokens[*count - 1] = (Token) {.type = TokenType_NO}; // last token is a null one
	return true;
}

void destroy_lexer(
Lexer* restrict lexer) {
	lexer->source = NULL;
	free(lexer->tokens);
	lexer->count = 0;
}
