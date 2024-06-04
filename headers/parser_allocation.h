#ifndef PARSER_ALLOCATION
#define PARSER_ALLOCATION

#include <stdlib.h>
#include "parser_def.h"

bool parser_create_allocator(Parser* parser);
bool parser_allocator(Parser* parser);
void parser_allocator_save(Parser* parser);
void parser_allocator_clear(Parser* parser);
void parser_allocator_restore(Parser* parser);
void parser_destroy_allocator(Parser* parser);

#endif
