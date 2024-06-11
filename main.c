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

	bool exit_status = true;
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

	if((exit_status = create_source(
		argv[1],
		&source))
	== false)
		goto END;

	printf("%s\n", source.content + 1);

	if((exit_status = create_memory_area(
		source.length,
		sizeof(uint8_t),
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
		&lexer)
	== false))
		goto END;
#ifndef NDEBUG
	debug_print_tokens(&lexer);
#endif
	if((exit_status = create_parser(
		&lexer,
		&memArea,
		&parser))
	== false)
		goto END;
#ifndef NDEBUG
	debug_print_declarations(&parser);
	debug_print_nodes(&parser);
#endif
	binary_x64(
		&binary,
		&parser);
END:
	destroy_parser(&parser);
	destroy_lexer(&lexer);
	destroy_binary(&binary);
	destroy_memory_area(&memArea);
	destroy_source(&source);
	return exit_status ? EXIT_SUCCESS : EXIT_FAILURE;
}
