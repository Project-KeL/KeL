#ifndef LEXER_ERROR_H
#define LEXER_ERROR_H

#include "allocator.h"
#include "source.h"

bool lexer_scan_errors(
	const Source* source,
	MemoryArea* memArea);

#endif
