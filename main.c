#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "allocator.h"
#include "assembly.h"
#include "debug_register.h"
#include "elf_binary.h"
#include "debug.h"
#include "tac.h"
#include "register.h"
#include "x64_mapping.h"

int main(
int argc,
char** argv) {
	if(argc != 2) {
		printf("Source file required.\n");
		return EXIT_FAILURE;
	}

	bool exit_status = true;
	Source source;
	MemoryArea area;
	Binary binary;
	Lexer lexer;
	Parser parser;
	TAC tac;
	RegMap regmap;
	Assembly assembly;

	initialize_source(&source);
	initialize_memory_area(&area); // for the lexer
	initialize_binary(&binary);
	initialize_lexer(&lexer);
	initialize_parser(&parser);
	initialize_tac(&tac);
	initialize_regmap(&regmap);
	initialize_assembly(&assembly);

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
		&area))
	== false)
		goto END;

	if((exit_status = create_binary(
		"./bin",
		&binary))
	== false)
		goto END;

	if((exit_status = create_lexer(
		&source,
		&area,
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
	debug_print_tree(&parser);
#endif
	if((exit_status = create_tac(
		&parser,
		&tac))
	== false)
		goto END;
#ifndef NDEBUG
	debug_print_quadruple_list(&tac);
#endif
	if((exit_status = create_regslots(
		16,
		512,
		&tac,
		&regmap.regslots))
	== false)
		goto END;
#ifndef NDEBUG
	debug_print_x64_register(&regmap.regslots);
#endif
	if((exit_status = create_assembly(
		"./EXE.sh",
		&tac.quadlist,
		&regmap,
		&assembly))
	== false)
		goto END;

	assembly_file_write(&assembly);
/*
	binary_x64(
		&binary,
		&parser);
*/
END:
	destroy_assembly(&assembly);
	destroy_regmap(&regmap);
	destroy_tac(&tac);
	destroy_parser(&parser);
	destroy_lexer(&lexer);
	destroy_binary(&binary);
	destroy_memory_area(&area);
	destroy_source(&source);
	return exit_status ? EXIT_SUCCESS : EXIT_FAILURE;
}
