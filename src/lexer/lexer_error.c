#include <assert.h>
#include "lexer_error.h"
#include "lexer_utils.h"

/*
 * 1 - a backslash must be followed by a letter or a digit
 * 2 - a lock qualifier cannot be the first token
 * 3 - scan delimiters matching
 * 4 - a colon cannot be followed by EOF
 * 5 - scan lonely colon
 * 6 - array and pointer check 
 * 7 - comments have to be well delimited
 */

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
	size_t open_delimiter_count = 0;
	long int i = 0;
	// lonely colon: avoid to test i != 0 in the loop
	if(source->content[i] == ':') {
		if(!is_significant(source->content[i + 1])
		|| source->content[i + 1] == '[')
			return false;
		else
			i += 1;
	}

	for(;
	!is_eof(code[i]);
	i += 1) {
		const char c = code[i];

		if(c == '\\') {
			if(is_eof(code[i + 1])
			|| !is_significant(code[i + 1]))
				return false;
			
			i += 1;
			continue;
		} else if(is_open_delimiter(c)) {
			allocator->last[open_delimiter_count] = c;
			open_delimiter_count += 1;
		} else if(is_close_delimiter(c)) {
			if(open_delimiter_count == 0)
				return false;

			if(delimiter_match(
				allocator->last[open_delimiter_count - 1],
				c)
			== false)
				return false;

			open_delimiter_count -= 1;
		} else if(c == ':') {
			if(is_eof(code[i + 1]))
				return false;

			if(!is_significant(code[i - 1])
			&& !is_significant(code[i + 1]))
				return false;

			if(!is_significant(code[i - 1])
			&& !is_alphabetical(code[i + 1])
			&& code[i + 1] != '('
			&& code[i + 1] != '['
			&& code[i + 1] != '&'
			&& code[i + 1] != '*')
				return false;
		} else if(!marker_literal_string
		       && c == '-') {
			if(is_eof(code[i + 1])
			|| code[i + 1] != '-')
				continue;

			i += 1;

			do {
				i += 1;
			} while(code[i] != '\n'
			     && !is_eof(code[i]));
		} else if(!marker_literal_string
		       && c == '|') {
			if(is_eof(code[i + 1])
			|| code[i + 1] != '-'
			|| is_eof(code[i + 2]
			|| code[i + 2] != '-'))
				continue;

			i += 2;

			do {
				i += 1;
			} while(!is_eof(code[i])
			     && code[i] != '-'
			     || (!is_eof(code[i + 1])
			      && code[i + 1] != '-')
				 || (!is_eof(code[i + 2])
				  && code[i + 2] != '|'));
			
			if(is_eof(code[i]))
				return false; // comment syntax error
		}

		if(c == '`')
			marker_literal_string = !marker_literal_string;
	}

	return open_delimiter_count == 0
	    && marker_literal_string == false;
}
