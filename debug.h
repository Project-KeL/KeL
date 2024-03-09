#ifndef DEBUG_H
#define DEBUG_H
#ifndef NDEBUG

#include "src/lexer/lexer.h"

void debug_print_tokens(const Lexer* lexer);

#endif
#endif
