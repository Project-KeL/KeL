#include <stdio.h>
#include <stdlib.h>
#include "source.h"

void initialize_source(Source* restrict source) {
	source->path = NULL;
	source->content = NULL;
	source->length = 0;
}

bool create_source(
const char* restrict path,
Source* restrict source) {
	if(source == NULL)
		return false;

	source->path = path;

	bool error = false;
	FILE* source_file = fopen(
		path,
		"r");

	if(source_file == NULL) {
		printf("Cannot open the source file at \"");
		printf(path);
		printf("\".\n");
		goto ERROR;
	}

	// get length
	fseek(
		source_file,
		0,
		SEEK_END);
	source->length = ftell(source_file);
	fseek(
		source_file,
		0,
		SEEK_SET);

	if(ferror(source_file) != 0) {
		printf("Error during a file positionning.\n");
		error = true;
		goto CLOSE;
	}
	// get source as a string
	source->content = malloc(source->length * sizeof(char) + 2);

	if(source->content == NULL) {
		printf("Allocation error.\n");
		error = true;
		goto CLOSE;
	}

	source->content[0] = '\0'; // will prevent errors with indexes
	if(fread(
		source->content + 1,
		1,
		source->length,
		source_file)
	!= (size_t) source->length) {
		printf("Cannot read the source at \"");
		printf(path);
		printf("\".\n");
		error = true;
	}
CLOSE:
	if(fclose(source_file) == EOF) {
		printf("Cannot close the source file at \"");
		printf(path);
		printf("\".\n");
	}
ERROR:
	if(error) {
		destroy_source(source);
		return false;
	}

	source->content[(size_t) source->length + 1] = '\0';
	return true;
}

void destroy_source(Source* restrict source) {
	if(source == NULL)
		return;

	free(source->content);
	source->path = NULL;
	source->content = NULL;
	source->length = 0;
}
