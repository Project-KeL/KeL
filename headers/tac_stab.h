#ifndef STAB_H
#define STAB_H

#include "parser.h"

typedef struct {
	size_t offset_node;
	uint32_t id;
} STabEntry;

typedef struct {
	Parser* parser;
	MemoryStack stack_entry;
	MemoryStack stack_watermark; // a stack of `MemoryStackState`
} STab; //Symbol Table

void initialize_tac_stab(STab* stab);
bool create_tac_stab(
	Parser* parser,
	STab* stab);
void destroy_tac_stab(STab* stab);
void tac_stab_push_entry(
	size_t offset_node,
	STab* stab);
void tac_stab_push_scope(STab* stab);
void tac_stab_pop_scope(STab* stab);
STabEntry* tac_stab_lookup(
	size_t offset_node,
	STab* stab);

#endif
