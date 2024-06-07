#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "parser_allocation.h"

#define CHUNK 1

bool parser_create_allocator(Parser* parser) {
	if(create_memory_chain(
		CHUNK,
		sizeof(Node),
		&parser->nodes)
	== false)
		return false;
	// null token implicit (calloc)
	return true;
}

bool parser_allocator(Parser* parser) {
	const size_t count = parser->nodes.last->memArea.count;

	if((Node*) parser->nodes.top + 1 >= (Node*) parser->nodes.last->memArea.addr + count) {
		// the remaining area is filled with null tokens (calloc)
		if(memory_chain_add_area(
			CHUNK,
			&parser->nodes)
		== false)
			return false;
	} else {
		parser->nodes.previous = parser->nodes.top;
		parser->nodes.top = (Node*) parser->nodes.top + 1;
	}

	return true;
}

#undef CHUNK
