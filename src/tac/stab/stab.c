#include <assert.h>
#include "allocator.h"
#include "parser_utils.h"
#include "stab.h"

void initialize_stab(STab* stab) {
	assert(stab != NULL);

	initialize_memory_stack(&stab->stack_entry);
	initialize_memory_stack(&stab->stack_watermark);
}

bool create_stab(
Parser* parser,
STab* stab) {
	assert(parser != NULL);
	assert(stab != NULL);

	if(!create_memory_stack(
		parser->nodes.count,
		sizeof(STabEntry),
		&stab->stack_entry)
	|| create_memory_stack(
		parser->nodes.count,
		sizeof(MemoryStackState),
		&stab->stack_watermark)
	== false)
		goto ERROR;

	stab->code = parser->lexer->source->content;
	// SCOPE_0
	MemoryStackState stack_state;
	initialize_memory_stack_state(&stack_state);
	memory_stack_state_save(
		&stab->stack_entry,
		&stack_state);
	memory_stack_push(
		(char*) &stack_state,
		&stab->stack_watermark);
	return true;
ERROR:
	destroy_stab(stab);
	return false;
}

void destroy_stab(STab* stab) {
	if(stab == NULL)
		return;

	destroy_memory_stack(&stab->stack_watermark);
	destroy_memory_stack(&stab->stack_entry);
	initialize_stab(stab);
}

void stab_push_entry(
STabEntry* stab_entry,
STab* stab) {
	memory_stack_push(
		(char*) stab_entry,
		&stab->stack_entry);
}

void stab_push_scope(STab* stab) {
	MemoryStackState stack_state;
	initialize_memory_stack_state(&stack_state);
	memory_stack_state_save(
		&stab->stack_entry,
		&stack_state);
	memory_stack_push(
		(char*) &stack_state,
		&stab->stack_watermark);
}

void stab_pop_scope(STab* stab) {
	assert(memory_stack_is_empty(&stab->stack_watermark) == false);

	MemoryStackState stack_state;
	memory_stack_pop(
		(char*) &stack_state,
		&stab->stack_watermark);
	memory_stack_state_restore(
		&stab->stack_entry,
		&stack_state);
}

STabEntry* stab_lookup(
const Token* token,
STab* stab) {
	STabEntry* const base = stab->stack_entry.area.base;
	STabEntry* entry = stab->stack_entry.top;

	while(entry != base) {
		entry -= 1;

		if(parser_is_code_token_match(
			stab->code,
			token,
			entry->token)
		== true)
			return entry;
	}

	return NULL;
}
