#include <assert.h>
#include "allocator.h"
#include "parser_utils.h"
#include "tac_stab.h"
#include <stdio.h>
void initialize_tac_stab(STab* stab) {
	assert(stab != NULL);

	initialize_memory_stack(&stab->stack_entry);
	initialize_memory_stack(&stab->stack_watermark);
}

bool create_tac_stab(
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

	STabEntry stab_entry = (STabEntry) {
		.offset_node = 0,
		.id = 0};
	memory_stack_push(
		(char*) &stab_entry,
		&stab->stack_entry);
	stab->parser = parser;
	return true;
ERROR:
	destroy_tac_stab(stab);
	return false;
}

void destroy_tac_stab(STab* stab) {
	if(stab == NULL)
		return;

	destroy_memory_stack(&stab->stack_watermark);
	destroy_memory_stack(&stab->stack_entry);
	initialize_tac_stab(stab);
}

void tac_stab_push_entry(
size_t offset_node,
STab* stab) {
	STabEntry* top_stab_entry = memory_stack_top_addr(&stab->stack_entry);
	STabEntry stab_entry = (STabEntry) {
		.offset_node = offset_node,
		.id = top_stab_entry->id + 1};
	memory_stack_push(
		(char*) &stab_entry,
		&stab->stack_entry);
}

void tac_stab_push_scope(STab* stab) {
	MemoryStackState stack_state;
	initialize_memory_stack_state(&stack_state);
	memory_stack_state_save(
		&stab->stack_entry,
		&stack_state);
	memory_stack_push(
		(char*) &stack_state,
		&stab->stack_watermark);
}

void tac_stab_pop_scope(STab* stab) {
	assert(memory_stack_is_empty(&stab->stack_watermark) == false);

	MemoryStackState stack_state;
	memory_stack_pop(
		(char*) &stack_state,
		&stab->stack_watermark);
	memory_stack_state_restore(
		&stab->stack_entry,
		&stack_state);
}

STabEntry* tac_stab_lookup(
size_t offset_node,
STab* stab) {
	STabEntry* const base = stab->stack_entry.area.base;
	STabEntry* entry = stab->stack_entry.top;
	Node* nodes = stab->parser->nodes.base;
	Token* tokens = stab->parser->lexer->tokens.base;

	while(entry != base) {
		entry -= 1;

		if(parser_is_code_token_match(
			stab->parser->lexer->source->content,
			tokens + nodes[offset_node].offset_token,
			tokens + nodes[entry->offset_node].offset_token)
		== true)
			return entry;
	}

	return NULL;
}
