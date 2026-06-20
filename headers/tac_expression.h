#ifndef TAC_EXPRESSION_H
#define TAC_EXPRESSION_H

#include <stddef.h>
#include "stab.h"
#include "tac.h"
#include "parser.h"
#include "tac_quadruple.h"

void tac_create_expression(
	size_t start,
	size_t end,
	size_t dst,
	MemoryStack* stack_buffer,
	TAC* tac);

#endif
