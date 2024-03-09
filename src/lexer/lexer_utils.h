#ifndef LEXER_UTILS_H
#define LEXER_UTILS_H

bool is_eof(char c);
bool is_significant(char c);
bool is_alphabetical(char c);
bool is_digit(char c);
bool is_open_delimiter(char c);
bool is_close_delimiter(char c);
bool is_delimiter(char c);
bool is_command(char c);
bool is_interpreted(char c);
bool is_special(char c);
bool delimiter_match(
	char c1,
	char c2);
bool is_valid_name(
	const char* restrict string,
	long int start,
	long int end);
bool skip_significant_but_not_special(
	const char* restrict string,
	long int* restrict end);
void skip_unsignificant_but_not_eof(
	const char* restrict string,
	long int* restrict end);
bool get_next_word_immediate(
	const char* restrict string,
	long int* restrict end);
bool get_next_word(
	const char* restrict string,
	long int* start,
	long int* end);

#endif
