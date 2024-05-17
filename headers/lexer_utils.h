#ifndef LEXER_UTILS_H
#define LEXER_UTILS_H

#include <ctype.h>

bool isXdigit(char c);
bool lexer_is_delimiter_open(char c);
bool lexer_is_delimiter_close(char c);
bool lexer_is_bracket(char c);
bool lexer_is_delimiter(char c);
bool lexer_is_command(char c);
bool lexer_is_interpreted(char c);
bool lexer_is_operator_leveling(char c);
bool lexer_is_operator_modifier(char c);
bool lexer_is_special(char c);
bool lexer_delimiter_match(
	char c1,
	char c2);
bool lexer_is_valid_name(
	const char* restrict string,
	long int start,
	long int end);
bool lexer_skip_glyphs_but_not_special(
	const char* restrict string,
	long int* restrict end);
void lexer_skip_controls_and_spaces_but_not_eof(
	const char* restrict string,
	long int* restrict end);
bool lexer_get_next_word_immediate(
	const char* restrict string,
	long int* restrict end);
bool lexer_get_next_word(
	const char* restrict string,
	long int* start,
	long int* end);
bool lexer_skip_comment(
	const char* code,
	long int* start,
	long int* end);

#endif
