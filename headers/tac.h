#ifndef TAC_H
#define TAC_H

#include <stdint.h>
#include "parser.h"
#include "stab.h"

typedef struct {
	STab stab;
} TAC;

void initialize_tac(TAC* tac);
bool create_tac(
	Parser* parser,
	TAC* tac);
void destroy_tac(TAC* tac);

#endif
