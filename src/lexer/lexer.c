#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "lexer.h"
#include "lexer_allocator.h"
#include "lexer_error.h"
#include "lexer_utils.h"

/*
 * LSPE: Left SPEcial character
 * RSPE: Right SPEcial character
 * COM: (COMmand) to declare`an identifier
 * ID: IDentifier, the declaration of a key
 * Q: Qualifier
 * LR: (Left-Right) an L, a colon and something immediately else, one after another
 * L: to the Left of a colon
 * R: to the Right of a colon
 * PL: Period-Left, a period immediatly followed by a name
 * LIT: LITeral
 * Colon lonely: when `:` is not surrounded, left and right
 *
 * Checking order:
 * 1 - COM
 * 2 - ID because it must detect a command and it is the only case
 * 3 - Q to get rid of this special case (words surrounded by brackets, surrounded by blanks)
 * 4 - LR it is the most common token
 * 5 - RSPE and R to process tokens with a colon, maybe an RSPE (or more) and an R
 *     RSCOPE (Right SCOPE) is mostly used for labels
 * 6 - L is also very common
 *     LSCOPE (Left SCOPE) like `then`
 * 7 - LIT
 * 8 - PL (special use of the period, so before specials)
 * 9 - LSPE, lonely colon and RSPE parenthesis nesting
 *
 * For `L:R`, `L` is an LR and `R`an R ; we read until there is no R or RSPE
 *
 * LSPE = TokenType_LSPE + lexer_character_to_subtype
 * RSPE = TokenType_RSPE + lexer_character_to_subtype
 * COM = TokenType_COM + lexer_character_to_subtype
 * ID =  TokenType_ID + TokenSubtype_NO
 * Q = TokenType_Q + TokenSubtype_NO
 * LR = TokenType_LR + TokenSubtype_NO
 * RSCOPE = TokenType_RSCOPE + TokenSubtype_NO
 * R = TokenType_R + TokenSubtype_NO
 * LSCOPE = TokenType_LSCOPE + TokenSubtype_NO
 * LSCOPE(then) = TokenType_LSCOPE + TokenSubtype_IF
 * LSCOPE(, ~then) = TokenType_LSCOPE + TokenSubtype_ELSE_IF
 * LSCOPE(~then) = TokenType_LSCOPE + TokenSubtype_ELSE
 * L = TokenType_L + TokenSubtype_NO
 * IMOD = TokenType_L + TokenSubtype_MODULE_INPUT
 * OMOD = TokenType_L + TokenSubtype_MODULE_OUTPUT
 * LIT = TokenType_LIT + (TokenSubtype_LIT_NUM / TokenSubtype_LIT_CHAR / TokenSubtype_LIT_STR)
 * PL = TokenType_PL + TokenSubtype_NO
 * COL = TokenType_COLON_LONELY + TokenSubtype_NO
 * 
 *
 * TODO: ID subtype will differentiate cases like `@a :A` and `a :@A
*/

// more errors will be supported in "lexer_error.c".
static int error = 0;

static int set_error(int value) {
	if(error == -1)
		return -1;

	error = value;
	return value;
}

static void create_token_LSPE(
const char* code,
long int start,
Token* token) {
	*token = (Token) {
		.type = TokenType_LSPE,
		.subtype = lexer_character_to_subtype(code[start]),
		.start = start,
		.end = start + 1};
}


static void create_token_RSPE(
const char* code,
long int start,
Token* token) {
	*token = (Token) {
		.type = TokenType_RSPE,
		.subtype = lexer_character_to_subtype(code[start]),
		.start = start,
		.end = start + 1};
}

static bool if_COM_create_token(
const char* code,
long int start,
Token* token) {
	if(!lexer_is_command(code[start]))
		return false;

	*(token) = (Token) {
		.type = TokenType_COM,
		.subtype = lexer_character_to_subtype(code[start]),
		.start = start,
		.end = start + 1};
	return true;
}

static bool if_ID_create_token(
const char* code,
long int start,
long int end,
size_t i,
Lexer* lexer) {
	Token* const tokens = lexer->tokens.base;

	if(tokens[i - 1].type != TokenType_COM
	|| lexer_is_name(
		code,
		start,
		end)
	== false)
		return false;

	tokens[i] = (Token) {
		.type = TokenType_ID,
		.subtype = TokenSubtype_NO,
		.start = start,
		.end = end};
	return true;
}

static int get_Q(
long int* restrict start,
long int* restrict end,
size_t* restrict i,
Lexer* lexer) {
	assert(start != end);

	const char* code = lexer->source->content;
	lexer_get_next_word(
		code,
		start,
		end);
	size_t buffer_i = *i;

	if(code[*start] == ']')
		return 0;

	do {
		Token* const tokens = lexer->tokens.base;
		tokens[buffer_i] = (Token) {
			.type = TokenType_Q,
			.subtype = TokenSubtype_NO,
			.start = *start,
			.end = *end};
		lexer_get_next_word(
			code,
			start,
			end);
		buffer_i += 1;
	} while(code[*end] != '\0'
	     && code[*start] != ']');

	if(isgraph(code[*end])) {
		return 0;
	}

	*i = buffer_i;
	return 1;
}

static int if_Q_create_tokens(
long int* restrict start,
long int* restrict end,
size_t* restrict i,
Lexer* lexer) {
	assert(start != end);

	const char* code = lexer->source->content;
	long int buffer_start = *start;
	long int buffer_end = *end;
	size_t buffer_i = *i;

	if(code[buffer_start] != '['
	|| isgraph(code[buffer_start - 1]))
		return 0;

	switch(get_Q(
		&buffer_start,
		&buffer_end,
		&buffer_i,
		lexer)) {
	case -1: return -1;
	case 0: return 0;
	case 1: /* fall through */;
	}

	*start = buffer_start;
	*end = buffer_end;
	*i = buffer_i - 1;
	return 1;
}

static bool if_LR_create_tokens(
long int start,
long int* end,
size_t i,
Lexer* lexer) {
	const char* code = lexer->source->content;
	Token* const tokens = lexer->tokens.base;

	if(code[*end] != ':'
	|| !isgraph(code[*end + 1])
	|| lexer_is_operator_modifier(code[start]))
		return false;

	tokens[i] = (Token) {
		.type = TokenType_LR,
		.subtype = TokenSubtype_NO,
		.start = start,
		.end = *end};
	return true;
}

static int if_RSPE_and_R_create_tokens(
long int start,
long int* end,
size_t* i,
Lexer* lexer) {
	const char* code = lexer->source->content;
	long int buffer_end = start + 1;
	size_t buffer_i = *i;

	if(code[start] != ':')
		return 0;

	lexer_get_next_word(
		code,
		&start,
		&buffer_end);

	if(lexer_is_operator_modifier(code[start])) {
		do {
			Token* const tokens = lexer->tokens.base;
			create_token_RSPE(
				code,
				start,
				tokens + buffer_i);
			lexer_get_next_word(
				code,
				&start,
				&buffer_end);
			buffer_i += 1;
		} while(lexer_is_operator_modifier(code[start]));
	} else if(code[start] == '_') {
		Token* const tokens = lexer->tokens.base;
		create_token_RSPE(
			code,
			start,
			tokens + buffer_i);
		*end = buffer_end;
		*i = buffer_i;
		return 1;
	}

	if(lexer_is_name(
		code,
		start,
		buffer_end)
	== false)
		return 0;

	*end = buffer_end;

	if(*end - start == 5 // as much as character than in `scope`
	&& strncmp(
		"scope",
		code + start,
		*end - start)
	== 0) {
		Token* const tokens = lexer->tokens.base;
		tokens[buffer_i] = (Token) {
			.type = TokenType_RSCOPE,
			.subtype = TokenSubtype_NO,
			.start = start,
			.end = *end};
	} else {
		Token* const tokens = lexer->tokens.base;
		tokens[buffer_i] = (Token) {
			.type = TokenType_R,
			.subtype = TokenSubtype_NO,
			.start = start,
			.end = *end};
	}

	*i = buffer_i;
	return 1;
}


static bool if_L_create_token(
long int start,
long int* end,
size_t* i,
Lexer* lexer) {
	const char* code = lexer->source->content;
	Token* const tokens = lexer->tokens.base;

	if(lexer_is_name(
		code,
		start,
		*end)
	== false)
		return false;

	if(*end - start == 5
	&& strncmp(
		"scope",
		code + start,
		*end - start)
	== 0) {
		tokens[*i] = (Token) {
			.type = TokenType_LSCOPE,
			.subtype = TokenSubtype_NO,
			.start = start,
			.end = *end};
	} else if(*end - start == 4
	       && strncmp(
		"then",
		code + start,
		*end - start)
	== 0) {
		TokenSubtype subtype = TokenSubtype_IF;

		if(code[tokens[*i - 1].start] == '~'
		&& *i > 1 // do better??
		&& code[tokens[*i - 2].start] == ',') {
			subtype = TokenSubtype_ELSE_IF;
			*i -= 2; // absorb `~` and `,`
		} else if(code[tokens[*i - 1].start] == '~') {
			subtype = TokenSubtype_ELSE;
			*i -= 1; // absorb `~`
		}

		tokens[*i] = (Token) {
			.type = TokenType_LSCOPE,
			.subtype = subtype,
			.start = start,
			.end = *end};
	} else if(*end - start == 4
	       && strncmp(
		"imod",
		code + start,
		*end - start)
	== 0) {
		tokens[*i] = (Token) {
			.type = TokenType_L,
			.subtype = TokenSubtype_MODULE_INPUT,
			.start = start,
			.end = *end};
	} else if(*end - start == 4
	       && strncmp(
		"omod",
		code + start,
		*end - start)
	== 0) {
		tokens[*i] = (Token) {
			.type = TokenType_L,
			.subtype = TokenSubtype_MODULE_OUTPUT,
			.start = start,
			.end = *end};
	} else {
		tokens[*i] = (Token) {
			.type = TokenType_L,
			.subtype = TokenSubtype_NO,
			.start = start,
			.end = *end};
	}

	return true;
}
// later: check the literal (for 0b0110, we have to check what follows `b`, it has to be `0` or `1`)
static bool if_LIT_create_token(
const char* code,
long int start,
long int* end,
Token* token) {
	TokenSubtype subtype;
	long int buffer_end = start + 1;

	if(isdigit(code[start])) {
		// base check
		if(code[start] == '0') {
			if(code[buffer_end] == 'b'
			 || code[buffer_end] == 'o'
			 || code[buffer_end] == 'x') {
				buffer_end += 1;
			} else {
				// 0 alone case
			}
		}

		while(code[buffer_end] != '\0'
		   && (isXdigit(code[buffer_end])
		    || code[buffer_end] == '`')) buffer_end += 1;
		// a number cannot be followed by '`' and must be followed by a blank or a special symbol
		if(code[buffer_end - 1] == '`'
		|| (isgraph(code[buffer_end])
		 && !lexer_is_special(code[buffer_end]))) {
			set_error(-1);
			return false;
		}

		subtype = TokenSubtype_LIT_NUM;
	} else if(code[start] == '\'') {
		while(code[buffer_end] != '\0'
		   && code[buffer_end] != '\'') buffer_end += 1;

		if(code[buffer_end] != '\'') {
			set_error(-1);
			return false;
		}

		start += 1;
		buffer_end += 1;
		subtype = TokenSubtype_LIT_CHAR;
	} else if(code[start] == '`') {
		while(code[buffer_end] != '\0'
		   && code[buffer_end] != '`') buffer_end += 1;

		if(code[buffer_end] != '`') {
			set_error(-1);
			return false;
		}

		start += 1;
		buffer_end += 1;
		subtype = TokenSubtype_LIT_STR;
	} else {
		return false;
	}

	*token = (Token) {
		.type = TokenType_LIT,
		.subtype = subtype,
		.start = start,
		.end = buffer_end - (subtype != TokenSubtype_LIT_NUM ? 1 : 0)};
	*end = buffer_end;
	return true;
}

static bool if_PL_create_token(
const char* code,
long int start,
long int* end,
Token* token) {
	if(code[start] != '.'
	|| !isgraph(code[start + 1]))
		return false;

	long int buffer_end = start + 1;
	lexer_get_next_word(
		code,
		&start,
		&buffer_end);

	if(lexer_is_name(
		code,
		start,
		buffer_end)
	== false)
		return false;

	*token = (Token) {
		.type = TokenType_PL,
		.subtype = TokenSubtype_NO,
		.start = start,
		.end = buffer_end};
	*end = buffer_end;
	return true;
}

void initialize_lexer(Lexer* lexer) {
	lexer->source = NULL;
	initialize_memory_area(&lexer->tokens);
}

bool create_lexer(
const Source* source,
MemoryArea* restrict memArea,
Lexer* lexer) {
	assert(source != NULL);
	assert(memArea != NULL);
	assert(lexer != NULL);

	error = 0;

	lexer->source = source;

	const char* code = source->content;
	long int start = 0;
	long int end = 1;
	long int count_RSPE_parenthesis_nest = 0;
	size_t i = 1;

	if(lexer_scan_errors(
		source,
		memArea)
	== false)
		return false;

	lexer_initialize_allocator(lexer);

	if(lexer_create_allocator_limit(
		lexer->source->length + 1,
		lexer)
	== false)
		goto DESTROY;

	while(lexer_get_next_word(
		code,
		&start,
		&end)
	== true) {
		while(lexer_skip_comment(
			code,
			&start,
			&end));

		if(code[end] == '\0')
			break;
		// create tokens
		Token* token = (Token*) lexer->tokens.base + i;

		if(if_COM_create_token(
			code,
			start,
			token)
		== true) {
			// OK
		} else if(if_ID_create_token(
			code,
			start,
			end,
			i,
			lexer)
		== true) {
			// OK
		} else if(set_error(
			if_Q_create_tokens(
				&start,
				&end,
				&i,
				lexer))
		== 1) {
			// OK
		} else if(if_LR_create_tokens(
			start,
			&end,
			i,
			lexer)
		== true)  {
			// OK
		} else if(set_error(
			if_RSPE_and_R_create_tokens(
				start,
				&end,
				&i,
				lexer))
		== 1) {
			if(code[end] == '(') {
				create_token_RSPE(
					code,
					end,
					token + 1);
				count_RSPE_parenthesis_nest += 1;
				end += 1;
				i += 1;
			}
		} else if(if_L_create_token(
			start,
			&end,
			&i,
			lexer)
		== true) {
			// OK
		} else if(if_PL_create_token(
			code,
			start,
			&end,
			token)
		== true) {
			// OK
		} else if(if_LIT_create_token(
			code,
			start,
			&end,
			token)
		== true) {
			// OK
		} else if(lexer_is_special(code[start])) {
			long int buffer_end = end;
			// modifiers on the left case
			if(lexer_is_operator_modifier(code[start])) {
				long int buffer_start = start;
				size_t buffer_i = i;
				// consume all the modifiers
				while(lexer_is_operator_modifier(code[buffer_start])) {
					lexer_get_next_word(
						code,
						&buffer_start,
						&buffer_end);
				}

				if(code[buffer_start] == ':') {
					do {
						Token* const tokens = lexer->tokens.base;
						create_token_RSPE(
							code,
							start,
							tokens + buffer_i);
						lexer_get_next_word(
							code,
							&start,
							&end);
						buffer_i += 1;
					} while(code[start] != ':');

					end -= 1;
					i = buffer_i - 1;
				} else {
					create_token_LSPE(
						code,
						start,
						token);
				}
			} else if(code[start] == ':') {
				if(code[start + 1] == '(') {
					create_token_RSPE(
						code,
						start + 1,
						token);
					end = start + 2;
					count_RSPE_parenthesis_nest += 1;
				} else {
					*token = (Token) {
						.type = TokenType_COLON_LONELY,
						.subtype = TokenSubtype_NO,
						.start = start,
						.end = start + 1};
				}
			} else if(code[start] == ')'
			       && count_RSPE_parenthesis_nest
			> 0) {
				create_token_RSPE(
					code,
					start,
					token);
				count_RSPE_parenthesis_nest -= 1;
			} else {
				create_token_LSPE(
					code,
					start,
					token);
			}
		} else
			goto DESTROY;

		if(error == -1)
			goto DESTROY;

		i += 1;
	}

	if(i == 1)
		goto DESTROY;

	lexer->tokens.count = i;

	if(!lexer_allocator_shrink(lexer))
		goto DESTROY;

	return true;
DESTROY:
	destroy_lexer(lexer);
	return false;
}

void destroy_lexer(Lexer* lexer) {
	if(lexer == NULL)
		return;
	
	lexer_destroy_allocator(lexer);
	initialize_lexer(lexer);
}
