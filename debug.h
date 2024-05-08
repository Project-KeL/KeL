#ifndef DEBUG_H
#define DEBUG_H
#ifndef NDEBUG

#include "lexer.h"
#include "parser.h"

void debug_print_tokens(const Lexer* lexer);
void debug_print_nodes(const Parser* parser);

#endif
#endif
