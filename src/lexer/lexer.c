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
 * 8 - is_period_key
 * 9 - is_literal
 * 10 - is_special
 * 11 - is_valid_name
 *
 * qualifier cases are checked first to detect the brackets, so it is easier to detect
 * names (for instance, the name of a key or a lock).
 */

// more errors will be implemented in the dedicated function
// this variable manages errors which subsequently may no longer be errors
static bool token_error = false;

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
		.subtype = subtype,
		.start = start,
		.end = end};
}

static void create_token_qualifier(
TokenType type,
TokenSubtype subtype,
long int start,
long int end,
Token* restrict token) {
	*token = (Token) {
		.type = type,
		.subtype = subtype,
		.start = start,
		.end = end};
}

static void create_token_colon_word(
TokenType type,
long int key_start,
long int key_end,
long int lock_start,
long int lock_end,
Token* restrict token) {
	*token = (Token) {
		.type = type,
		.subtype = TokenSubtype_NO,
		.key_start = key_start,
		.key_end = key_end,
		.lock_start = lock_start,
		.lock_end = lock_end};
}

static void create_token_literal(
TokenSubtype subtype,
long int start,
long int end,
Token* token) {
	*token = (Token) {
		.type = TokenType_LITERAL,
		.subtype = subtype,
		.start = start,
		.end = end};
}

static bool is_valid_qualifier_key(
const char* code,
TokenSubtype* subtype,
long int start,
long int end) {
	*subtype = TokenSubtype_NO;
	code = &code[start]; // for strcmp

	if(strcmp(
		"entry",
		code)
	<= 0)
		*subtype = TokenSubtype_QUALIFIER_KEY_ENTRY;
	else if(strcmp(
		"mut",
		code)
	<= 0)
		*subtype = TokenSubtype_QUALIFIER_KEY_MUT;

	return *subtype != TokenSubtype_NO;
}

static bool is_valid_qualifier_lock(
const char* code,
TokenSubtype* subtype,
long int start,
long int end) {
	*subtype = TokenSubtype_NO;
	code = &code[start];

	if(strcmp(
		"default",
		code)
	<= 0)
		*subtype = TokenSubtype_QUALIFIER_LOCK_DEFAULT;

	return *subtype != TokenSubtype_NO;
}

static bool if_command_create_token(
const char* code,
long int start,
Token* token) {
	if(!is_command(code[start]))
		return false;

	create_token_special(
		code,
		start,
		start + 1,
		token);
	return true;
}

static bool get_qualifier_key(
const char* restrict code,
TokenSubtype* subtype,
long int start,
long int* end,
long int* key_start,
long int* key_end) {
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
		subtype,
		start,
		buffer_end)
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

	*end = buffer_end;
	return true;
}

static bool if_qualifier_key_create_token(
const char* restrict code,
long int start,
long int* end,
Token* token) {
	TokenSubtype subtype;
	long int buffer_end = *end;
	long int key_start;
	long int key_end;

	if(!get_qualifier_key(
		code,
		&subtype,
		start,
		&buffer_end,
		&key_start,
		&key_end)
	|| (is_significant(code[buffer_end])
	 && code[buffer_end] != ':') // syntax error here
	|| is_significant(code[buffer_end + 1]))
		return false;

	*end = buffer_end;

	if(code[buffer_end] == ':')
		*end += 1;

	create_token_qualifier(
		TokenType_QUALIFIER_KEY,
		subtype,
		key_start,
		key_end,
		token);
	return true;
}

static bool if_key_create_token(
bool previous_is_command,
const char* restrict code,
long int start,
long int* end,
Token* token) {
	if(previous_is_command
	|| (is_significant(code[*end])
	 && code[*end] != ':')
	|| is_valid_name(
		code,
		start,
		*end)
	== false)
		return false;

	create_token_colon_word(
		TokenType_KEY,
		start,
		*end,
		*end,
		*end,
		token);
	
	if(code[*end] == ':')
		*end += 1;

	return true;
}

static bool get_qualifier_lock(
const char* restrict code,
TokenSubtype* subtype,
long int start,
long int* end,
long int* lock_start,
long int* lock_end) {
	if(code[start] != ':'
	|| code[start + 1] != '[')
		return false;

	start += 1;
	long int buffer_end = start + 1;
	get_next_word(
		code,
		&start,
		&buffer_end);

	if(is_valid_qualifier_lock(
		code,
		subtype,
		start,
		buffer_end)
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

static bool if_qualifier_lock_create_token(
const char* restrict code,
long int start,
long int* end,
Token* token) {
	TokenSubtype subtype;
	long int buffer_end = *end;
	long int lock_start;
	long int lock_end;

	if(!get_qualifier_lock(
		code,
		&subtype,
		start,
		&buffer_end,
		&lock_start,
		&lock_end)
	|| is_significant(code[buffer_end]))
		return false;

	*end = buffer_end;
	create_token_qualifier(
		TokenType_QUALIFIER_LOCK,
		subtype,
		lock_start,
		lock_end,
		token);
	return true;
}

static bool if_lock_create_token(
const char* restrict code,
long int start,
long int* end,
Token* token) {
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
	create_token_colon_word(
		TokenType_LOCK,
		start,
		start,
		start,
		*end,
		token);
	return true;
}

static bool if_qualifier_key_lock_create_token(
const char* restrict code,
long int start,
long int* end,
Token* token) {
	TokenSubtype subtype1;
	TokenSubtype subtype2;
	long int buffer_end = *end;
	long int key_start;
	long int key_end;
	long int lock_start;
	long int lock_end;
	
	if(get_qualifier_key(
		code,
		&subtype1,
		start,
		&buffer_end,
		&key_start,
		&key_end)
	== false)
		return false;

	start = buffer_end;

	if(get_qualifier_lock(
		code,
		&subtype2,
		start,
		&buffer_end,
		&lock_start,
		&lock_end)
	== false)
		return false;

	if(is_significant(code[buffer_end]))
		return false;

	*end = buffer_end;
	*token = (Token) {
		.type = TokenType_QUALIFIER_KEY_LOCK,
		.subtype = subtype1 | subtype2,
		.key_start = key_start,
		.key_end = key_end,
		.lock_start = lock_start,
		.lock_end = lock_end};
	return true;
}

static bool if_keyword_create_token(
bool previous_is_command,
const char* restrict code,
long int start,
long int* end,
Token* token) {
	if(previous_is_command
	|| !is_valid_name(
		code,
		start,
		*end)
	|| code[*end] != ':')
		return false;

	long int lock_start = *end + 1;
	long int lock_end = *end + 1;
	get_next_word(
		code,
		&lock_start,
		&lock_end);

	if(is_valid_name(
		code,
		lock_start,
		lock_end)
	== false)
		return false;

	create_token_colon_word(
		TokenType_KEYWORD,
		start,
		*end,
		lock_start,
		lock_end,
		token);
	*end = lock_end;
	return true;
}

static bool if_delimiter_create_token(
const char* code,
long int start,
Token* token) {
	if(!is_delimiter(code[start]))
		return false;

	create_token_special(
		code,
		start,
		start + 1,
		token);
	return true;
}

static bool if_literal_create_token(
const char* restrict code,
long int start,
long int* end,
Token* token) {
	TokenSubtype subtype;
	long int buffer_end = start + 1;

	if(is_digit(code[start])) {
		if(code[start] != '0'
		|| is_digit(code[buffer_end]))
			goto NO_BASE_CHECK;

		switch(code[buffer_end]) {
			case 'b': break;
			case 'o': break;
			case 'x': break;
			default: token_error = true; return false; // unknown base
		}

		buffer_end += 1;

		if(!is_digit_hex(code[buffer_end])) {
			token_error = true;
			return false;
		}
NO_BASE_CHECK:
		while(!is_eof(code[buffer_end]) 
		   && (is_digit_hex(code[buffer_end])
		    || code[buffer_end] == '`')) buffer_end += 1;
		// a number cannot be followed by '`' and must be followed by a blank or a special symbole
		if(code[buffer_end - 1] == '`'
		|| (is_significant(code[buffer_end])
		 && !is_special(code[buffer_end]))) {
			token_error = true;
			return false;
		}

		subtype = TokenSubtype_LITERAL_NUMBER;
	} else if(code[start] == '`') {
		while(!is_eof(code[buffer_end])
		   && code[buffer_end] != '`') buffer_end += 1;

		if(code[buffer_end] != '`') {
			token_error = true;
			return false;
		}

		buffer_end += 1;
		subtype = TokenSubtype_LITERAL_STRING;
	} else if(code[start] == '\\') {
		if(!is_alphabetical(code[buffer_end])
		&& !is_digit(code[buffer_end])) {
			token_error = true;
			return false;
		}

		buffer_end += 1;
		subtype = TokenSubtype_LITERAL_ASCII;
	} else
		return false;

	*end = buffer_end;
	*token = (Token) {
		.type = TokenType_LITERAL,
		.subtype = subtype,
		.start = start,
		.end = *end};
	return true;
}

static bool if_period_key_create_token(
const char* code,
long int start,
long int* end,
Token* token) {
	if(code[start] != '.'
	|| is_eof(code[start + 1])
	|| !is_significant(code[start + 1]))
		return false;

	long int buffer_end = start + 1;
	get_next_word(
		code,
		&start,
		&buffer_end);

	if(is_valid_name(
		code,
		start,
		buffer_end)
	== false)
		return false;

	*end = buffer_end;
	create_token_colon_word(
		TokenType_PERIOD_KEY,
		start + 1,
		buffer_end,
		buffer_end,
		buffer_end,
		token);
	return true;
}

static bool if_special_create_token(
const char* code,
long int start,
Token* token) {
	if(!is_special(code[start]))
		return false;

	create_token_special(
		code,
		start,
		start + 1,
		token);
	return true;
}

static bool if_valid_name_create_token(
const char* code,
long int start,
long int end,
Token* token) {
	if(is_valid_name(
		code,
		start,
		end)
	== false)
		return false;

	*token = (Token) {
		.type = TokenType_IDENTIFIER,
		.subtype = TokenSubtype_NO,
		.start = start,
		.end = end};
	return true;
}

bool create_lexer(
const Source* source,
Allocator* restrict allocator,
Lexer* restrict lexer) {
	lexer->source = source;
	lexer->tokens = NULL;
	lexer->count = 0;

	const char* code = source->content;
	bool previous_is_command = false; // checked for keys only
	long int start = 0;
	long int end = 0;
	long int i = 0;

	if(lexer_scan_errors(
		source,
		allocator)
	== false)
		return false;
#define TOKENS_CHUNK 4096
	while(get_next_word(
		code,
		&start,
		&end)
	== true) {
		while(skip_comment(
			code,
			&start,
			&end)) continue;

		if(is_eof(code[start]))
			break;
		// allocation
		if(lexer->count <= i) {
			Token* tokens_realloc = realloc(
				lexer->tokens,
				(lexer->count + TOKENS_CHUNK) * sizeof(Token));

			if(tokens_realloc == NULL) {
				destroy_lexer(lexer);
				return false;
			}
			
			lexer->tokens = tokens_realloc;
			lexer->count += TOKENS_CHUNK;
		}
		// create tokens
		Token* token = &lexer->tokens[i];

		if(if_command_create_token(
			code,
			start,
			token)
		== true) {
			previous_is_command = true;
		} else if(if_qualifier_key_create_token(
			code,
			start,
			&end,
			token)
		== true) {
			// OK
		} else if(if_key_create_token(
			previous_is_command,
			code,
			start,
			&end,
			token)
		== true)  {
			// OK
		} else if(if_qualifier_lock_create_token(
			code,
			start,
			&end,
			token)
		== true) {
			// OK
		} else if(if_lock_create_token(
			code,
			start,
			&end,
			token)
		== true) {
			// OK
		} else if(if_qualifier_key_lock_create_token(
			code,
			start,
			&end,
			token)
		== true) {
			// OK
		} else if(if_keyword_create_token(
			previous_is_command,
			code,
			start,
			&end,
			token)
		== true) {
			// OK
		} else if(if_period_key_create_token(
			code,
			start,
			&end,
			token)
		== true) {
			// OK

		} else if(if_literal_create_token(
			code,
			start,
			&end,
			token)
		== true) {
			// OK
		} else if(if_special_create_token(
			code,
			start,
			token)
		== true) {
			// OK
		} else if(if_valid_name_create_token(
			code,
			start,
			end,
			token)
		== true) {
			// OK
		} else {
			destroy_lexer(lexer);
			return false;
		}

		previous_is_command = is_command(code[start]);
		start = end;
		i += 1;
	}

	lexer->count = i + 1; // null token
	Token* tokens_realloc = realloc(
		lexer->tokens,
		lexer->count * sizeof(Token));
#undef TOKENS_CHUNK // no more allocation reminder
	if(tokens_realloc == NULL) {
		destroy_lexer(lexer);
		return false;
	}

	lexer->tokens = tokens_realloc;
	lexer->tokens[lexer->count - 1] = (Token) {
		.type = TokenType_NO,
		.subtype = TokenSubtype_NO,
		.start = 0,
		.end = 0}; // last token is a null one
	return true;
}

void destroy_lexer(
Lexer* restrict lexer) {
	lexer->source = NULL;
	free(lexer->tokens);
	lexer->count = 0;
}
