#ifndef PARSER_ALLOCATION
#define PARSER_ALLOCATION

#include <stdlib.h>
#include "parser_def.h"

void parser_initialize_allocators(Parser* parser);
bool parser_create_allocators(Parser* parser);
void parser_destroy_allocators(Parser* parser);
bool parser_allocator_node(Parser* parser);
bool parser_allocator_declaration(Parser* parser);

#endif
