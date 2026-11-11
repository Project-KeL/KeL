#ifndef TAC_H
#define TAC_H

#include <stdint.h>
#include "parser.h"
#include "tac_stab.h"
#include "tac_quadruple.h"

typedef struct {
	STab stab;
	QuadList quadlist;
	size_t offset_entry;
} TAC;

static inline TAC setup_tac(void) {
	return (TAC) {
		.stab = setup_tac_stab(),
		.quadlist = setup_quadlist()};
}
bool create_tac(
	Parser* parser,
	TAC* tac);
void destroy_tac(TAC* tac);

#endif
