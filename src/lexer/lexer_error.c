#include <assert.h>
#include "lexer_error.h"
#include "lexer_utils.h"

#include <stdio.h>

bool lexer_scan_errors(
const Source* restrict source,
Allocator* restrict allocator) {
	assert(
		allocator_fit(
			allocator,
			source->length)); // at least the size of the source (matching parenthesis)
	const char* code = source->content;
	bool marker_literal_string = false;
	size_t count_delimiter_open = 0;
	long int start = 0;
	long int end = 0;
	// lonely colon: avoid to test start != 0 in the loop
	if(source->content[start] == ':') {
		if(!isgraph(source->content[start + 1])
		|| source->content[start + 1] == '[')
			return false;
		else
			start += 1;
	}

	while(lexer_get_next_word(
		code,
		&start,
		&end)
	== true) {
		// cannot call skip_comment
		const char c = code[start];
		// LITERAL_ASCII_NO
		if(c == '\\'
		&& !isgraph(code[start + 1])) {
			return false;
		// DELIMITER_MATCH
		} else if(lexer_is_delimiter_open(c)) {
			allocator->last[count_delimiter_open] = c;
			count_delimiter_open += 1;
		} else if(lexer_is_delimiter_close(c)) {
			if(count_delimiter_open == 0)
				return false;

			if(lexer_delimiter_match(
				allocator->last[count_delimiter_open - 1],
				c)
			== false)
				return false;

			count_delimiter_open -= 1;
		// COLON
		} else if(c == ':') {
			// COLON_EOF
			if(code[start + 1] == '\0')
				return false;
			// COLON_RIGHT_COLON
			if(code[start + 1] == ':')
				return false;
			// COLON_LONELY
			if(!isgraph(code[start - 1])
			 && !isgraph(code[start + 1]))
				return false;
			// COLON_LONELY_RIGHT_ALONE_LEFT
			if(!isgraph(code[start - 1])
			&& !lexer_is_command(code[start + 1])
			&& !isalpha(code[start + 1]) // an R begins with a letter
			&& code[start + 1] != '('
			&& code[start + 1] != '['
			&& code[start + 1] != '&')
				return false;
		// process a comment
		} else if(!marker_literal_string
		       && c == '!') {
			if(code[start + 1] != '-'
			|| code[start + 2] != '-')
				continue;

			start += 2;

			do {
				start += 1;
			} while(code[start] != '\n'
			     && code[start] != '\0');
		// COMMENT_MULTILINE_TRAIL_NO
		} else if(!marker_literal_string
		       && c == '|') {
			if(code[start + 1] != '-'
			|| code[start + 2] != '-')
				continue;

			start += 2;

			do {
				start += 1;
			} while(code[start] != '\0'
			     && code[start] != '-'
			     || (code[start + 1] != '\0'
			      && code[start + 1] != '-')
				 || (code[start + 2] != '\0'
				  && code[start + 2] != '|'));

			if(code[start] == '\0')
				return false;
		// KEY_MODIFIER_EOF
		} else if(lexer_is_special(c)) {
			while(lexer_is_operator_leveling(code[end])
			   || lexer_is_bracket(code[end])) {
				if(code[end] == '\0')
					return false;

				end += 1;
			}
		}

		if(c == '`')
			marker_literal_string = !marker_literal_string;
	}

	return count_delimiter_open == 0
	    && marker_literal_string == false;
}
