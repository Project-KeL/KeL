#include <stdio.h>
#include <stdlib.h>
#include "debug.h"
#include "kel.h"

int main(
int argc,
char** argv) {
	if(argc != 2) {
		printf("Source file required.\n");
		return EXIT_FAILURE;
	}

	bool error = false;
	Source source;
	Allocator allocator;
	Binary binary;
	Lexer lexer;
	Parser parser;

	if(create_source(
		argv[1],
		&source)
	== false) {
		error = true;
		goto ERROR_0;
	}

	printf("%s\n", source.content + 1);

	if(create_allocator(
		(size_t) source.length,
		&allocator)
	== false) {
		error = true;
		goto ERROR_1;
	}

	if(create_binary(
		"./bin",
		&binary)
	== false) {
		error = true;
		goto ERROR_2;
	}

	printf("%s\n", source.content);

	if(create_lexer(
		&source,
		&allocator,
		&lexer)
	== false) {
		error = true;
		goto ERROR_3;
	}
#ifndef NDEBUG
	debug_print_tokens(&lexer);
#endif
	if(create_parser(
		&lexer,
		&allocator,
		&parser)
	== false) {
		error = true;
		goto ERROR_4;
	}
#ifndef NDEBUG
	debug_print_nodes(&parser);
#endif
	binary_x64(
		&binary,
		&parser);
	destroy_parser(&parser);
ERROR_4:
	destroy_lexer(&lexer);
ERROR_3:
	destroy_binary(&binary);
ERROR_2:
	destroy_allocator(&allocator);
ERROR_1:
	destroy_source(&source);
ERROR_0:
	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
