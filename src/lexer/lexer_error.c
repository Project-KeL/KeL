#include <assert.h>
#include "lexer_error.h"
#include "lexer_utils.h"

/*
 * 1 - a lock qualifier cannot be the first token
 * 2 - scan delimiters matching
 * 3 - a colon cannot be followed by EOF
 * 4 - scan lonely colon
 * 5 - array and pointer check 
 * 6 - comments have to be well delimited
 */

#include <stdio.h>
bool lexer_scan_errors(
const Source* restrict source,
Allocator* restrict allocator) {
	assert(
		allocator_fit(
			allocator,
			source->length)); // at least the size of the source (matching parenthesis)

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
	!is_eof(source->content[i]);
	i += 1) {
		const char c = source->content[i];

		if(is_open_delimiter(c)) {
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
			if(is_eof(source->content[i + 1]))
				return false;

			if(!is_significant(source->content[i - 1])
			&& !is_significant(source->content[i + 1]))
				return false;

			if(!is_significant(source->content[i - 1])
			&& !is_alphabetical(source->content[i + 1])
			&& source->content[i + 1] != '('
			&& source->content[i + 1] != '['
			&& source->content[i + 1] != '&'
			&& source->content[i + 1] != '*')
				return false;
		} else if(c == '|') {
			if(!is_eof(source->content[i + 1])
			&& source->content[i + 1] != '-')
				continue;

			i += 1;

			do {
				i += 1;
			} while(!is_eof(source->content[i])
			     && (source->content[i] != '-'
			      || (!is_eof(source->content[i + 1])
			       && source->content[i + 1] != '|')));
			
			if(is_eof(source->content[i])
			|| source->content[i] != '-'
			|| is_eof(source->content[i + 1])
			|| source->content[i + 1] != '|')
				return false; // comment syntax error
		}
	}

	return open_delimiter_count == 0;
}
