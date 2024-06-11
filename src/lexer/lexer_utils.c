#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include "lexer_utils.h"

TokenSubtype lexer_character_to_subtype(char c) {
	switch(c) {
	case '!': return TokenSubtype_EXCLAMATION_MARK;
	case '"': return TokenSubtype_DQUOTES;
	case '#': return TokenSubtype_HASH;
	case '%': return TokenSubtype_MODULO;
	case '&': return TokenSubtype_AMPERSAND;
	case '\'': return TokenSubtype_SQUOTE;
	case '(': return TokenSubtype_LPARENTHESIS;
	case ')': return TokenSubtype_RPARENTHESIS;
	case '*': return TokenSubtype_ASTERISK;
	case '+': return TokenSubtype_PLUS;
	case ',': return TokenSubtype_COMMA;
	case '-': return TokenSubtype_MINUS;
	case '.': return TokenSubtype_PERIOD;
	case '/': return TokenSubtype_DIVIDE;
	case ':': return TokenSubtype_COLON;
	case ';': return TokenSubtype_SEMICOLON;
	case '<': return TokenSubtype_LOBRACKET;
	case '=': return TokenSubtype_EQUAL;
	case '>': return TokenSubtype_ROBRACKET;
	case '?': return TokenSubtype_QUESTION_MARK;
	case '@': return TokenSubtype_AT;
	case '[': return TokenSubtype_LBRACKET;
	case ']': return TokenSubtype_RBRACKET;
	case '\\': return TokenSubtype_BACKSLASH;
	case '^': return TokenSubtype_CARET;
	case '`': return TokenSubtype_GRAVE_ACCENT;
	case '{': return TokenSubtype_LCBRACE;
	case '|': return TokenSubtype_PIPE;
	case '}': return TokenSubtype_RCBRACE;
	case '~': return TokenSubtype_TILDE;
	default: assert(false); // missing case
	}
}

bool isXdigit(char c) {
	return isdigit(c)
	    || (c >= 'A'
		 && c <= 'F');
}

bool lexer_is_delimiter_open(char c) {
	return c == '('
	    || c == '['
	    || c == '{';
}

bool lexer_is_delimiter_close(char c) {
	return c == ')'
	    || c == ']'
	    || c == '}';
}

bool lexer_is_parenthesis(char c) {
	return c == '('
	    || c == ')';
}

bool lexer_is_bracket(char c) {
	return c == '['
	    || c == ']';
}

bool lexer_is_delimiter(char c) {
	return lexer_is_delimiter_open(c)
	    || lexer_is_delimiter_close(c);
}

bool lexer_is_command(char c) {
	return c == '!'
	    || c == '#'
	    || c == '@';
}
// is interpreted when encountered alone
bool lexer_is_interpreted(char c) {
	return lexer_is_delimiter(c)
	    || lexer_is_command(c)
	    || c == '!'
	    || c == '"'
	    || c == '%'
	    || c == '&'
	    || c == '\''
	    || c == '*'
	    || c == '+'
	    || c == ','
	    || c == '-'
	    || c == '.'
	    || c == '/'
	    || c == ';'
	    || c == '<'
	    || c == '='
	    || c == '>'
	    || c == '?'
	    || c == '\\'
	    || c == '^'
	    || c == '`'
	    || c == '|'
	    || c == '~';
}

bool lexer_is_operator_leveling(char c) {
	return c == '&'
	    || c == '+'
	    || c == '-'
	    || c == '|';
}

bool lexer_is_operator_modifier(char c) {
	return lexer_is_operator_leveling(c)
	    || lexer_is_bracket(c);
}

bool lexer_is_operator(char c) {
	return c == '%'
	    || c == '&'
	    || c == '*'
	    || c == '+'
	    || c == '-'
	    || c == '/'
	    || c == '^'
	    || c == '|'
	    || c == '~';
}

bool lexer_is_special(char c) {
	return lexer_is_interpreted(c) || c == ':';
}

bool lexer_delimiter_match(
char c1,
char c2) {
	if(c1 == '('
	&& c2 == ')')
		return true;
	else if(c1 == '['
	     && c2 == ']')
		return true;
	else if(c1 == '{'
	     && c2 == '}')
		return true;
	else
		return false;
}
// name of a colon word
bool lexer_is_valid_name(
const char* string,
long int start,
long int end) {
	assert(string != NULL);

	string = string + start; // because start is left untouched
	bool is_valid = isalpha(*string);

	for(long int i = 1;
	i < end - start;
	i += 1) {
		const char c = string[i];
		is_valid = is_valid
			&& (isalpha(c)
			 || isdigit(c)
		     || c == '_');
	}

	return is_valid;
}

bool lexer_skip_glyphs_but_not_special(
const char* string,
long int* end) {
	if(!isgraph(string[*end])
	|| lexer_is_special(string[*end]))
		return false;

	do {
		*end += 1;
	} while(isgraph(string[*end])
	     && !lexer_is_special(string[*end]));

	return true;
}

void lexer_skip_controls_and_spaces_but_not_eof(
const char* string,
long int* end) {
	while(string[*end] != '\0'
	   && !isgraph(string[*end])) *end += 1;
}

bool lexer_get_next_word_immediate(
const char* string,
long int* end) {
	if(string[*end] == '\0')
		return false;
	else if(lexer_is_special(string[*end])) {
		*end += 1;
		return true;
	} else
		return lexer_skip_glyphs_but_not_special(
			string,
			end);
}

bool lexer_get_next_word(
const char* string,
long int* restrict start,
long int* restrict end) {
	assert(start != NULL);
	assert(end != NULL);
	assert(start != end);

	lexer_skip_controls_and_spaces_but_not_eof(
		string,
		end);
	*start = *end;

	return lexer_get_next_word_immediate(
		string,
		end);
}

bool lexer_skip_comment(
const char* string,
long int* restrict start,
long int* restrict end) {
	assert(start != NULL);
	assert(end != NULL);
	assert(start != end);

	if(string[*start] == '!') {
		if(string[*start + 1] != '-'
		|| string[*start + 2] != '-')
			return false;

		*end += 1;

		do {
			*end += 1;
		} while(string[*end] != '\n'
			 && string[*end] != '\0');
	} else if(string[*start] == '|') {
		if(string[*start + 1] != '-'
		|| string[*start + 2] != '-')
			return false;

		*end += 1;

		do {
			*end += 1;
		} while(string[*end] != '-'
		   || string[*end + 1] != '-'
		   || string[*end + 2] != '|'); // error checked
			
		*end += 3;
	} else
		return false;

	lexer_get_next_word(
		string,
		start,
		end);
	return true;
}
