#include <stdio.h>
#include <stdlib.h>
#include "src/exl.h"

int main(
int argc,
char** argv) {
	if(argc != 2) {
		printf("Source file required.\n");
		return EXIT_FAILURE;
	}

	bool error = false;
	Source source;
	Lexer lexer;

	if(create_source(
		argv[1],
		&source)
	== false) {
		error = true;
		goto ERROR_0;
	}

	printf("%s\n\n", source.content);

	if(create_lexer(
		&source,
		&lexer)
	== false) {
		error = true;
		goto ERROR_1;
	}

	destroy_lexer(&lexer);
ERROR_1:
	destroy_source(&source);
ERROR_0:
	if(error) exit(EXIT_FAILURE);
	return EXIT_SUCCESS;
}
