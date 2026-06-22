#ifndef TAC_H
#define TAC_H

#include <stdint.h>
#include "parser.h"
#include "tac_stab.h"
#include "tac_quadruple.h"

typedef struct {
	STab stab;
	QuadList quadlist;
} TAC;

void initialize_tac(TAC* tac);
bool create_tac(
	Parser* parser,
	TAC* tac);
void destroy_tac(TAC* tac);

#endif
