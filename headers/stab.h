#ifndef STAB_H
#define STAB_H

#include "parser.h"

typedef struct {
	const Token* token;
	uint32_t id;
} STabEntry;

typedef struct {
	const char* code;
	MemoryStack stack_entry;
	MemoryStack stack_watermark; // a stack of `MemoryStackState`
} STab; //Symbol Table

void initialize_stab(STab* stab);
bool create_stab(
	Parser* parser,
	STab* stab);
void destroy_stab(STab* stab);
void stab_push_entry(
	const Token* token,
	STab* stab);
void stab_push_scope(STab* stab);
void stab_pop_scope(STab* stab);

#endif
