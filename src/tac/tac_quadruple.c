#include <assert.h>
#include "tac_quadruple.h"

void initialize_quadlist(QuadList* quadruple_list) {
	assert(quadruple_list != NULL);

	initialize_memory_stack(&quadruple_list->quadruples);
}

bool create_quadlist(
Parser* parser,
QuadList* quadruple_list) {
	assert(parser != NULL);
	assert(quadruple_list != NULL);

	if(create_memory_stack(
		parser->lexer->source->length,
		sizeof(QuadEntry),
		&quadruple_list->quadruples)
	== false)
		return false;

	return true;
}

void destroy_quadlist(QuadList* tac_quadruples) {
	if(tac_quadruples == NULL)
		return;

	destroy_memory_stack(&tac_quadruples->quadruples);
	initialize_quadlist(tac_quadruples);
}

QuadItem create_quaditem_null(void) {
	return (QuadItem) {
		.type = QuadItemType_NO,
		.offset_node = 0};
}

void quadlist_append(
QuadEntry* entry,
QuadList* quadruple_list) {
	assert(entry != NULL);
	assert(quadruple_list != NULL);

	memory_stack_push(
		(char*) entry,
		&quadruple_list->quadruples);
}
