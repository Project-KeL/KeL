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

	bool error = false;
	FILE* source_file = fopen(
		path,
		"r");

	if(source_file == NULL) {
		error = true;
		goto END;
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
		error = true;
		goto END;
	}
	// get source as a string and add sentinel at start and at end
	source->content = malloc(source->length * sizeof(char) + 2);

	if(source->content == NULL) {
		error = true;
		goto END;
	}

	source->content[0] = '\0'; // will prevent errors with indexes

	if(fread(
		source->content + 1,
		1,
		source->length,
		source_file)
	!= (size_t) source->length) {
		error =true;
		goto END;
	}
END:
	if(fclose(source_file) == EOF)
		error = true;

	source->content[(size_t) source->length + 1] = '\0';

	if(error == true) {
		destroy_source(source);
		return false;
	}

	return true;
}

void destroy_source(Source* source) {
	if(source == NULL)
		return;

	free(source->content);
	initialize_source(source);
}
