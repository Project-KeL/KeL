#ifndef PARSER_ALLOCATION
#define PARSER_ALLOCATION

#include <stdlib.h>
#include "parser_def.h"

bool parser_allocate_chunk(
	size_t minimum,
	Parser* parser);

#endif
