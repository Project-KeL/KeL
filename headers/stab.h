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

void initialize_stab(STab* stab);
bool create_stab(
	Parser* parser,
	STab* stab);
void destroy_stab(STab* stab);
void stab_push_entry(
	size_t offset_node,
	STab* stab);
void stab_push_scope(STab* stab);
void stab_pop_scope(STab* stab);
STabEntry* stab_lookup(
	size_t offset_node,
	STab* stab);

#endif
