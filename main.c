#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "allocator.h"
#include "debug.h"
#include "kel.h"

int main(
int argc,
char** argv) {
	if(argc != 2) {
		printf("Source file required.\n");
		return EXIT_FAILURE;
	}

	bool exit_status = true;
	Source source;
	MemoryArea memArea;
	Binary binary;
	Lexer lexer;
	Parser parser;
	// TAC tac;
	initialize_source(&source);
	initialize_memory_area(&memArea); // for the lexer
	initialize_binary(&binary);
	initialize_lexer(&lexer);
	initialize_parser(&parser);
	// initialize_TAC(&tac);

	if((exit_status = create_source(
		argv[1],
		&source))
	== false)
		goto END;

	if((exit_status = source.length == 0))
		goto END;

	printf("%s\n", source.content + 1);

	if((exit_status = create_memory_area(
		source.length,
		sizeof(union {
			uintmax_t uintmax;
			void* ptr;
			void (*fn)(void);
			size_t size;}),
		&memArea))
	== false)
		goto END;

	if((exit_status = create_binary(
		"./bin",
		&binary))
	== false)
		goto END;

	if((exit_status = create_lexer(
		&source,
		&memArea,
		&lexer))
	== false)
		goto END;
#ifndef NDEBUG
	debug_lexer_print_tokens(&lexer);
#endif
	if((exit_status = create_parser(
		&lexer,
		&parser))
	== false)
		goto END;
#ifndef NDEBUG
	debug_print_nodes(&parser);
#endif
/*
	binary_x64(
		&binary,
		&parser);
*/
END:
	destroy_parser(&parser);
	destroy_lexer(&lexer);
	destroy_binary(&binary);
	destroy_memory_area(&memArea);
	destroy_source(&source);
	return exit_status ? EXIT_SUCCESS : EXIT_FAILURE;
}
