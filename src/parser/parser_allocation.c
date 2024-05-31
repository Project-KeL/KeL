#include <stdlib.h>
#include <stdio.h>
#include "parser_allocation.h"

bool parser_allocate_chunk(
long int minimum,
Parser* parser) {
#define NODES_CHUNK 4096
	if(parser->count <= minimum) {
		const long int reserve = minimum / NODES_CHUNK + 1;
		Node* nodes_realloc = realloc(
			parser->nodes,
			reserve * NODES_CHUNK * sizeof(Node));

		if(nodes_realloc == NULL)
			return false;

		parser->nodes = nodes_realloc;
		parser->count = reserve * NODES_CHUNK;
	}

	return true;
#undef NODES_CHUNK
}
