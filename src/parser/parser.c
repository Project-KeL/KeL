#include "parser.h"

void initialize_parser(Parser* parser) {
	assert(parser != NULL);

	parser->lexer = NULL;
	initialize_memory_chain(&parser->nodes);
}

void destroy_parser(Parser* parser) {
	if(parser == NULL)
		return;

	MemoryChainIterator iterator;
	start_memory_chain_iterator(
		&parser->nodes,
		&iterator);
	end_memory_chain_iterator(&iterator);
	initialize_parser(parser);
}
