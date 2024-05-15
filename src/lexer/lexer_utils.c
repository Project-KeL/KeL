#include <assert.h>
#include <stddef.h>
#include "lexer_utils.h"
#include <stdio.h>

bool isXdigit(char c) {
	return isdigit(c)
	    || (c >= 'A'
		 && c <= 'F');
}

bool is_delimiter_open(char c) {
	return c == '('
	    || c == '['
	    || c == '{';
}

bool is_delimiter_close(char c) {
	return c == ')'
	    || c == ']'
	    || c == '}';
}

bool is_bracket(char c) {
	return c == '['
	    || c == ']';
}

bool is_delimiter(char c) {
	return is_delimiter_open(c)
	    || is_delimiter_close(c);
}

bool is_command(char c) {
	return c == '#'
	    || c == '@';
}
// is interpreted when encountered alone
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

bool is_operator_leveling(char c) {
	return c == '&'
	    || c == '+'
	    || c == '-'
	    || c == '|';
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
	bool is_valid = isalpha(string[0]);

	for(long int i = 1;
	i < end - start;
	i += 1) {
		const char c = string[i];
		is_valid = is_valid
			&& (isalpha(c)
			 || isdigit(c));
	}

	return is_valid;
}

bool skip_significant_but_not_special(
const char* restrict string,
long int* restrict end) {
	if(!isgraph(string[*end])
	|| is_special(string[*end]))
		return false;

	do {
		*end += 1;
	} while(isgraph(string[*end])
	     && !is_special(string[*end]));

	return true;
}

void skip_unsignificant_but_not_eof(
const char* restrict string,
long int* restrict end) {
	while(string[*end] != '\0'
	   && !isgraph(string[*end])) *end += 1;
}

bool get_next_word_immediate(
const char* restrict string,
long int* restrict end) {
	if(string[*end] == '\0')
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

bool skip_comment(
const char* code,
long int* start,
long int* end) {
	if(code[*start] == '!') {
		if(code[*start + 1] != '-'
		|| code[*start + 2] != '-')
			return false;

		*end += 2;

		do {
			*end += 1;
		} while(code[*end] != '\n'
			 && code[*end] != '\0');
	} else if(code[*start] == '|') {
		if(code[*start + 1] != '-'
		|| code[*start + 2] != '-')
			return false;

		*end += 2;

		do {
			*end += 1;
		} while(code[*end] != '-'
		   || code[*end + 1] != '-'
		   || code[*end + 2] != '|'); // error checked
			
		*end += 2;
	} else
		return false;

	get_next_word(
		code,
		start,
		end);
	return true;
}
