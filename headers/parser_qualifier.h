#ifndef PARSER_QUALIFIER_H
#define PARSER_QUALIFIER_H

#include "parser.h"

bool if_GRP_Q_create_operator(
	size_t* i,
	size_t* j,
	MemoryStack* stack_context,
	MemoryStack* stack_operator,
	Parser* parser);

#endif
