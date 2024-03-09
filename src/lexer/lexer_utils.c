#include <assert.h>
#include <stddef.h>
#include "lexer_utils.h"

bool is_eof(char c) {
	return c == '\0';
}

bool is_significant(char c) {
	return c > 32 && c < 127;
}

bool is_alphabetical(char c) {
	return c > 64 && c < 91
	    || c > 96 && c < 123;
}

bool is_digit(char c) {
	return c > 47 && c < 58;
}

bool is_open_delimiter(char c) {
	return c == '('
	    || c == '['
		|| c == '{';
}

bool is_close_delimiter(char c) {
	return c == ')'
	    || c == ']'
		|| c == '}';
}

bool is_delimiter(char c) {
	return is_open_delimiter(c)
	    || is_close_delimiter(c);
}

bool is_command(char c) {
	return c == '#'
	    || c == '@';
}
// is interpreted when encounter alone
bool is_interpreted(char c) {
	return is_delimiter(c)
	    || is_command(c)
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

bool is_operator(char c) {
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

bool is_special(char c) {
	return is_interpreted(c) || c == ':';
}

bool delimiter_match(
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
bool is_valid_name(
const char* restrict string,
long int start,
long int end) {
	string = &string[start]; // because start is left untouched
	bool is_valid = is_alphabetical(string[0]);

	for(long int i = 1;
	i < end - start;
	i += 1) {
		const char c = string[i];
		is_valid = is_valid
			&& (is_alphabetical(c)
			 || is_digit(c));
	}

	return is_valid;
}

bool skip_significant_but_not_special(
const char* restrict string,
long int* restrict end) {
	if(!is_significant(string[*end])
	|| is_special(string[*end]))
		return false;

	do {
		*end += 1;
	} while(is_significant(string[*end])
	     && !is_special(string[*end]));

	return true;
}

void skip_unsignificant_but_not_eof(
const char* restrict string,
long int* restrict end) {
	while(!is_eof(string[*end])
	   && !is_significant(string[*end])) *end += 1;
}

bool get_next_word_immediate(
const char* restrict string,
long int* restrict end) {
	if(is_eof(string[*end]))
		return false;
	else if(is_special(string[*end])) {
		*end += 1;
		return true;
	} else
		return skip_significant_but_not_special(
			string,
			end);
}

bool get_next_word(
const char* restrict string,
long int* start,
long int* end) {
	assert(start != NULL);
	assert(end != NULL);
	skip_unsignificant_but_not_eof(
		string,
		end);
	*start = *end;

	return get_next_word_immediate(
		string,
		end);
}
