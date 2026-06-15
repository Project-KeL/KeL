#include <assert.h>
#include "tac_quadruple.h"

void initialize_quadruple_list(QuadrupleList* quadruple_list) {
	assert(quadruple_list != NULL);

	initialize_memory_stack(&quadruple_list->quadruples);
}

bool create_quadruple_list(
Parser* parser,
QuadrupleList* quadruple_list) {
	assert(parser != NULL);
	assert(quadruple_list != NULL);

	if(create_memory_stack(
		parser->lexer->source->length,
		sizeof(QuadrupleEntry),
		&quadruple_list->quadruples)
	== false)
		return false;

	return true;
}

void destroy_quadruple_list(QuadrupleList* tac_quadruples) {
	if(tac_quadruples == NULL)
		return;

	destroy_memory_stack(&tac_quadruples->quadruples);
	initialize_quadruple_list(tac_quadruples);
}

void quadruple_list_append(
QuadrupleEntry* entry,
QuadrupleList* quadruple_list) {
	assert(entry != NULL);
	assert(quadruple_list != NULL);

	memory_stack_push(
		(char*) entry,
		&quadruple_list->quadruples);
}
