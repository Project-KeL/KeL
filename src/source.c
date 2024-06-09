#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "source.h"

void initialize_source(Source* source) {
	source->path = NULL;
	source->content = NULL;
	source->length = 0;
}

bool create_source(
const char* restrict path,
Source* restrict source) {
	assert(path != NULL);
	assert(source != NULL);

	source->path = path;

	bool error = true;
	FILE* source_file = fopen(
		path,
		"r");

	if(source_file == NULL)
		goto ERROR;
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

	if(ferror(source_file) != 0)
		goto CLOSE;
	// get source as a string
	source->content = malloc(source->length * sizeof(char) + 2);

	if(source->content == NULL)
		goto CLOSE;

	source->content[0] = '\0'; // will prevent errors with indexes

	if(fread(
		source->content + 1,
		1,
		source->length,
		source_file)
	!= (size_t) source->length)
		goto CLOSE;

	error = false;
CLOSE:
	if(fclose(source_file) == EOF)
		error = true;

	if(error)
		goto ERROR;

	source->content[(size_t) source->length + 1] = '\0';
	return true;
ERROR:
	destroy_source(source);
	return false;
}

void destroy_source(Source* source) {
	if(source == NULL)
		return;

	free(source->content);
	initialize_source(source);
}
