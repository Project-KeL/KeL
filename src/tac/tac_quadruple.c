#include <assert.h>
#include "tac_quadruple.h"

static void create_quadentry_null(QuadEntry* entry) {
	*entry = (QuadEntry) {
		.op = setup_quaditem(),
		.src1 = setup_quaditem(),
		.src2 = setup_quaditem(),
		.dst = setup_quaditem()};
}

bool quadlist_allocator_shrink_append_null(QuadList* quadlist) {
	assert(quadlist != NULL);

	MemoryStack* stack = &quadlist->quadruples;
	size_t count = ((char*) stack->top - (char*) stack->area.base) / stack->area.size_type;
	const bool error = memory_stack_realloc(
		count + 1, // null token
		&quadlist->quadruples);
	// sentinel
	QuadEntry entry;
	create_quadentry_null(&entry);
	memory_stack_push(
		(char*) &entry,
		&quadlist->quadruples);
	return error;
}

bool create_quadlist(
Parser* parser,
QuadList* quadlist) {
	assert(parser != NULL);
	assert(quadlist != NULL);

	if(create_memory_stack(
		parser->lexer->source->length + 1,
		sizeof(QuadEntry),
		&quadlist->quadruples)
	== false)
		return false;

	QuadEntry entry;
	create_quadentry_null(&entry);
	memory_stack_push(
		(char*) &entry,
		&quadlist->quadruples);
	return true;
}

void destroy_quadlist(QuadList* tac_quadruples) {
	if(tac_quadruples == NULL)
		return;

	destroy_memory_stack(&tac_quadruples->quadruples);
	*tac_quadruples = setup_quadlist();
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
