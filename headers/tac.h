#ifndef TAC_H
#define TAC_H

#include <stdint.h>
#include "allocator.h"
#include "lexer.h"

typedef struct {
	//
} TAC;

void initialize_TAC(TAC* tac);
bool create_TAC(
	const Lexer* lexer,
	MemoryArea* restrict memArea,
	TAC* parser);
void destroy_TAC(TAC* tac);

#endif
