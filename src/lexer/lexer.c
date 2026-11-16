#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "lexer.h"
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
 * TODO: ID subtype will differentiate cases like `@a :A` and `a :@A` (left and right)
*/

static int error = 0;

static int set_error(int value) {
	if(error == -1)
		return -1;

	error = value;
	return value;
}

static Token make_token_null(void) {
	return (Token) {};
}

static bool lexer_create_allocator_limit(
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

	*((Token*) lexer->tokens.base) = make_token_null();
	return true;
}

static bool lexer_allocator_shrink_append_null(Lexer* lexer) {
	// at most one token per character, plus the null token at start
	assert(lexer->tokens.count <= (size_t) lexer->source->length + 1);

	if(memory_area_realloc(
		lexer->tokens.count + 1, // null token
		&lexer->tokens)
	== false)
		return false;
	// for the parser
	*((Token*) lexer->tokens.base + lexer->tokens.count - 1) = make_token_null();
	return true;
}

static void lexer_destroy_allocator(Lexer* lexer) {
	if(lexer == NULL)
		return;

	destroy_memory_area(&lexer->tokens);
}

static bool lexer_delimiter_update(
const char* code,
Token* token,
MemoryStack* restrict stack_delimiter) {
	const char c = code[token->start];

	if(lexer_is_delimiter_open(c)) {
		return memory_stack_push(
			(char*) &token,
			stack_delimiter);
	}

	if(lexer_is_delimiter_close(c)) {
		const Token* token_open;

		if(memory_stack_pop(
			(char*) &token_open,
			stack_delimiter)
		== false)
			return false; // nothing to close

		token->type = token_open->type; // LSPE or RSPE as its delimiter
		return lexer_is_delimiter_match(
			code[token_open->start],
			c);
	}

	return true;
}

static Token make_token_LSPE(
const char* code,
long int start) {
	return (Token) {
		.type = TokenType_LSPE,
		.subtype = lexer_character_to_subtype(code[start]), // missing case checked
		.start = start,
		.end = start + 1};
}

static Token make_token_RSPE(
const char* code,
long int start) {
	return (Token) {
		.type = TokenType_RSPE,
		.subtype = lexer_character_to_subtype(code[start]),
		.start = start,
		.end = start + 1};
}

static bool if_COM_emit_token(
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

static bool if_ID_emit_token(
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

static Token make_token_Q(
long int start,
long int end) {
	return (Token) {
		.type = TokenType_Q,
		.subtype = TokenSubtype_NO,
		.start = start,
		.end = end};
}

static bool emit_Qs(
long int* restrict start,
long int* restrict end,
size_t* restrict i,
Lexer* lexer) {
	const char* code = lexer->source->content;
	lexer_get_next_word(
		code,
		start,
		end);
	size_t buffer_i = *i;

	if(code[*start] == ']')
		return false;

	do {
		if(lexer_is_name(
			code,
			*start,
			*end)
		== false)
			return false;

		Token* const tokens = lexer->tokens.base;
		tokens[buffer_i] = make_token_Q(
			*start,
			*end);
		lexer_get_next_word(
			code,
			start,
			end);

		if(code[*end] == '\0')
			return false;

		buffer_i += 1;
	} while(code[*start] != ']');

	if(isgraph(code[*end])) {
		return false;
	}

	*i = buffer_i;
	return true;
}

static int if_Qs_emit_tokens(
long int* restrict start,
long int* restrict end,
size_t* restrict i,
Lexer* lexer) {
	const char* code = lexer->source->content;
	long int buffer_start = *start;
	long int buffer_end = *end;
	size_t buffer_i = *i;

	if(code[buffer_start] != '['
	|| isgraph(code[buffer_start - 1]))
		return 0;

	if(emit_Qs(
		&buffer_start,
		&buffer_end,
		&buffer_i,
		lexer)
	== false)
		return false;

	*start = buffer_start;
	*end = buffer_end;
	*i = buffer_i - 1;
	return 1;
}

static bool if_LR_emit_tokens(
long int start,
long int* end,
size_t i,
Lexer* lexer) {
	const char* code = lexer->source->content;
	Token* const tokens = lexer->tokens.base;

	if(code[*end] != ':'
	|| !isgraph(code[*end + 1])
	// a special character is never at the left of a LR
	|| lexer_is_special(code[start]))
		return false;

	tokens[i] = (Token) {
		.type = TokenType_LR,
		.subtype = TokenSubtype_NO,
		.start = start,
		.end = *end};
	return true;
}

static int if_RSPE_and_R_emit_tokens(
long int start,
long int* end,
size_t* i,
Lexer* lexer) {
	const char* code = lexer->source->content;
	Token* const tokens = lexer->tokens.base;
	long int buffer_end = start + 1;
	size_t buffer_i = *i;
	// the brackets of the modifiers must be closed among them
	long int count_bracket_open = 0;
	bool bracket_alone = false;

	if(code[start] != ':')
		return 0;

	if(code[start - 1] == ':')
		return -1; // two side by side colons is not permitted

	lexer_get_next_word(
		code,
		&start,
		&buffer_end);

	if(lexer_is_operator_modifier(code[start])) {
		do {
			tokens[buffer_i] = make_token_RSPE(
				code,
				start);

			if(code[start] == '[') {
				count_bracket_open += 1;
			} else if(code[start] == ']') {
				if(count_bracket_open == 0)
					bracket_alone = true;
				else
					count_bracket_open -= 1;
			}

			lexer_get_next_word(
				code,
				&start,
				&buffer_end);
			buffer_i += 1;
		} while(lexer_is_operator_modifier(code[start]));
	} else if(code[start] == '_') {
		/*
		Token* const tokens = lexer->tokens.base;
		emit_token_RSPE(
			code,
			start,
			tokens + buffer_i);
		*end = buffer_end;
		*i = buffer_i;
		return 1;
		*/
	}

	if(lexer_is_name(
		code,
		start,
		buffer_end)
	== false)
		return 0;

	if(bracket_alone
	|| count_bracket_open != 0)
		return -1;

	*end = buffer_end;

	if(*end - start == 5 // as much as character than in `scope`
	&& strncmp(
		"scope",
		code + start,
		*end - start)
	== 0) {
		tokens[buffer_i] = (Token) {
			.type = TokenType_RSCOPE,
			.subtype = TokenSubtype_NO,
			.start = start,
			.end = *end};
	} else {
		tokens[buffer_i] = (Token) {
			.type = TokenType_R,
			.subtype = TokenSubtype_NO,
			.start = start,
			.end = *end};
	}

	*i = buffer_i;
	return 1;
}


static bool if_L_emit_token(
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
		const bool tilde_before = tokens[*i - 1].type == TokenType_LSPE
		                       && tokens[*i - 1].subtype == TokenSubtype_TILDE;
		// the first token is null, if `i - 1` is a tilde, it must be at least at `i - 2`
		const size_t comma_i = tilde_before ? *i - 2 : *i - 1;
		const bool comma_before = tokens[comma_i].type == TokenType_LSPE
		                       && tokens[comma_i].subtype == TokenSubtype_COMMA;
		TokenSubtype subtype;

		if(comma_before) {
			subtype = tilde_before ? TokenSubtype_ELSE_IF : TokenSubtype_IF;
			*i = comma_i;
		} else if(tilde_before) {
			subtype = TokenSubtype_ELSE;
			*i -= 1; // absorb `~`
		} else {
			set_error(-1);
			return false;
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
			.subtype = TokenSubtype_IMOD,
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
			.subtype = TokenSubtype_OMOD,
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
static bool if_LIT_emit_token(
const char* code,
long int start,
long int* end,
Token* token) {
	TokenSubtype subtype;
	long int buffer_end = start + 1;

	if(isdigit(code[start])) {
#define EXHAUST(is_base) \
		while(code[buffer_end] != '\0' \
		   && (is_base(code[buffer_end]) \
		    || code[buffer_end] == '`')) buffer_end += 1;
		// base check
		if(code[start] == '0') {
			switch(code[buffer_end]) {
				case 'b':
					buffer_end += 1;
					EXHAUST(isBdigit);
					break;
				case 'o':
					buffer_end += 1;
					EXHAUST(isOdigit);
					break;
				case 'x':
					buffer_end += 1;
					EXHAUST(isXdigit);
					break;
				default:
					break;
			}
		// a base must be followed by at least one digit
			if(buffer_end == start + 2) {
				set_error(-1);
				return false;
			}
		} else {
			EXHAUST(isdigit);
		}
		// a number cannot be followed by '`' and must be followed by a blank or a special symbol
		if(code[buffer_end - 1] == '`'
		|| (isgraph(code[buffer_end])
		 && !lexer_is_special(code[buffer_end]))) {
			set_error(-1);
			return false;
		}
#undef EXHAUST
		subtype = TokenSubtype_LIT_NUM;
	} else if(code[start] == '\\') {
		if(code[start + 1] == '\0'
		|| code[start + 2] != '\\') {
			set_error(-1);
			return false;
		}

		buffer_end = start + 3;
		start += 1;
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

static bool if_PL_emit_token(
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

static bool if_special_emit_tokens(
const char* code,
long int start,
long int* end,
size_t* i,
MemoryStack* stack_delimiter,
Token* tokens) {
	if(!lexer_is_special(code[start]))
		return false;
	// modifiers on the left case
	if(lexer_is_operator_modifier(code[start])) {
		long int buffer_start = start;
		long int buffer_end = *end;
		long int buffer_i = *i;
		// consume all the modifiers
		while(lexer_is_operator_modifier(code[buffer_start])) {
			lexer_get_next_word(
				code,
				&buffer_start,
				&buffer_end);
		}
		// the modifiers are RSPEs
		if(code[buffer_start] == ':') {
			do {
				tokens[buffer_i] = make_token_RSPE(
					code,
					start);

				if(lexer_delimiter_update(
					code,
					tokens + buffer_i,
					stack_delimiter)
				== false)
					set_error(-1);

				lexer_get_next_word(
					code,
					&start,
					end);
				buffer_i += 1;
			} while(code[start] != ':');
			// process the colon
			*end -= 1;
			*i = buffer_i - 1;
		// no `:`, the modifier is an LSPE
		} else {
			tokens[*i] = make_token_LSPE(
				code,
				start);

			if(lexer_delimiter_update(
				code,
				tokens + *i,
				stack_delimiter)
			== false)
				set_error(-1);
		}
	} else if(code[start] == ':') {
		if(lexer_is_delimiter_open(code[start + 1])) {
			tokens[*i] = make_token_RSPE(
				code,
				start + 1);

			if(lexer_delimiter_update(
				code,
				tokens + *i,
				stack_delimiter)
			== false)
				set_error(-1);

			*end = start + 2;
		} else {
			tokens[*i] = (Token) {
				.type = TokenType_COLON_LONELY,
				.subtype = TokenSubtype_NO,
				.start = start,
				.end = start + 1};
		}
	} else {
		tokens[*i] = make_token_LSPE(
			code,
			start);

		if(lexer_delimiter_update(
			code,
			tokens + *i,
			stack_delimiter)
		== false)
			set_error(-1);
	}

	return true;
}

bool create_lexer(
const Source* source,
MemoryStack* stack_delimiter,
Lexer* lexer) {
	assert(source != NULL);
	assert(stack_delimiter != NULL);
	assert(lexer != NULL);
	assert(stack_delimiter->area.size_type == sizeof(Token*));

	error = 0;

	lexer->source = source;

	const char* code = source->content;
	long int start = 0;
	long int end = 1;
	size_t i = 1;

	if(lexer_create_allocator_limit(
		lexer->source->length + 1,
		lexer)
	== false)
		goto DESTROY;

	Token* tokens = lexer->tokens.base;

	while(lexer_get_next_word(
		code,
		&start,
		&end)
	== true) {
		// comments
		{
			int comment;

			while((comment = lexer_skip_comment(
				code,
				&start,
				&end)) == 1);

			if(comment == -1)
				set_error(-1);
		}

		if(code[start] == '\0')
			break;
		// create tokens
		if(if_COM_emit_token(
			code,
			start,
			tokens + i)
		== true) {
			// OK
		} else if(if_ID_emit_token(
			code,
			start,
			end,
			i,
			lexer)
		== true) {
			// OK
		} else if(set_error(
			if_Qs_emit_tokens(
				&start,
				&end,
				&i,
				lexer))
		== 1) {
			// OK
		} else if(if_LR_emit_tokens(
			start,
			&end,
			i,
			lexer)
		== true)  {
			// OK
		} else if(set_error(
			if_RSPE_and_R_emit_tokens(
				start,
				&end,
				&i,
				lexer))
		== 1) {
			if(code[end] == '(') {
				i += 1;
				tokens[i] = make_token_RSPE(
					code,
					end);

				if(lexer_delimiter_update(
					code,
					tokens + i,
					stack_delimiter)
				== false)
					set_error(-1);

				end += 1;
			}
		} else if(if_L_emit_token(
			start,
			&end,
			&i,
			lexer)
		== true) {
			// OK
		} else if(if_PL_emit_token(
			code,
			start,
			&end,
			tokens + i)
		== true) {
			// OK
		} else if(if_LIT_emit_token(
			code,
			start,
			&end,
			tokens + i)
		== true) {
			// OK
		} else if(if_special_emit_tokens(
			code,
			start,
			&end,
			&i,
			stack_delimiter,
			tokens)
		== true) {
			// OK
		} else
			goto DESTROY;

		if(error == -1)
			goto DESTROY;

		i += 1;
	}
	// a delimiter is not closed
	if(!memory_stack_is_empty(stack_delimiter)
	// no tokens but the first null token
	|| i == 1)
		goto DESTROY;

	lexer->tokens.count = i;

	if(!lexer_allocator_shrink_append_null(lexer))
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
	*lexer = setup_lexer();
}
