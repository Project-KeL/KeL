#include <stdio.h>
#include <stdint.h>
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
	MemoryArea memArea;
	Binary binary;
	Lexer lexer;
	Parser parser;

	initialize_source(&source);
	initialize_memory_area(&memArea);
	initialize_binary(&binary);
	initialize_lexer(&lexer);
	initialize_parser(&parser);

	if(create_source(
		argv[1],
		&source)
	== false) {
		error = true;
		goto ERROR_0;
	}

	if(create_memory_area(
		source.length,
		sizeof(uint8_t),
		&memArea)
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
		&memArea,
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
		&memArea,
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
	destroy_memory_area(&memArea);
ERROR_1:
	destroy_source(&source);
ERROR_0:
	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
