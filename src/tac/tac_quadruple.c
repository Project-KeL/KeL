#include <assert.h>
#include "tac_quadruple.h"

void initialize_quadruple(QuadrupleList* tac_quadruple) {
	assert(tac_quadruple != NULL);

	initialize_memory_area(&tac_quadruple->quadruples);
}

bool create_quadruples(
Parser* parser,
QuadrupleList tac_quadruple) {
	if(create_memory_area(
		parser->lexer->source->length,
		sizeof(QuadrupleEntry),
		&tac_quadruple.quadruples)
	== false)
		return false;

	return true;
}

void destroy_quadruples(QuadrupleList* tac_quadruples) {
	if(tac_quadruples == NULL)
		return;

	destroy_memory_area(&tac_quadruples->quadruples);
	initialize_quadruple(tac_quadruples);
}
