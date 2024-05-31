#include <assert.h>
#include "lexer_error.h"
#include "lexer_utils.h"

#include <stdio.h>

bool lexer_scan_errors(
const Source* restrict source,
MemoryArea* restrict memArea) {
	assert(memArea->size >= (size_t) source->length); // at least the size of the source (matching parenthesis)

	const char* code = source->content;
	char* const memory = memArea->addr;
	bool marker_literal_string = false;
	size_t count_delimiter_open = 0;
	long int start = 0;
	long int end = 1;
	// a source begins with a null character so that checking code[start - 1] is valid	
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
			memory[count_delimiter_open] = c;
			count_delimiter_open += 1;
		} else if(lexer_is_delimiter_close(c)) {
			if(count_delimiter_open == 0)
				return false;

			if(lexer_delimiter_match(
				memory[count_delimiter_open - 1],
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
			// COLON_VALID_SPECIAL_LEFT
			if(lexer_is_special(code[start - 1])
			&& code[start - 1] != '#'
			&& code[start - 1] != '&'
			&& code[start - 1] != '(' // parameterized labels
			&& code[start - 1] != ')'
			&& code[start - 1] != '+'
			&& code[start - 1] != '-'
			&& code[start - 1] != '@'
			&& code[start - 1] != ']'
			&& code[start - 1] != '|')
				return false;
			// COLON_VALID_SPECIAL_RIGHT
			if(lexer_is_special(code[start + 1])
			&& code[start + 1] != '#'
			&& code[start + 1] != '&'
			&& code[start + 1] != '('
			&& code[start + 1] != '+'
			&& code[start + 1] != '-'
			&& code[start + 1] != '@'
			&& code[start + 1] != '['
			&& code[start + 1] != '`'
			&& code[start + 1] != '|')
				return false;
		// process a comment
		} else if(!marker_literal_string
		       && c == '!') {
			if(code[start + 1] != '-'
			|| code[start + 2] != '-')
				continue;

			end += 1;

			do {
				end += 1;
			} while(code[end] != '\n'
			     && code[end] != '\0');
		// COMMENT_MULTILINE_TRAIL_NO
		} else if(!marker_literal_string
		       && c == '|') {
			if(code[start + 1] != '-'
			|| code[start + 2] != '-')
				continue;

			end += 1;

			do {
				end += 1;
			} while((code[end] != '\0'
			     && code[end] != '-')
			     || (code[end + 1] != '\0'
			      && code[end + 1] != '-')
				 || (code[end + 2] != '\0'
				  && code[end + 2] != '|'));

			if(code[end] == '\0')
				return false;

			end += 2;
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
