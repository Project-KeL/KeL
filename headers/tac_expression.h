#ifndef TAC_EXPRESSION_H
#define TAC_EXPRESSION_H

#include <stddef.h>
#include "stab.h"
#include "parser.h"

void tac_create_expression(
	size_t start,
	size_t end,
	const STab* stab);

#endif
