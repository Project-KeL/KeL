#ifndef LEXER_ERROR_H
#define LEXER_ERROR_H

#include "allocator.h"
#include "source.h"

bool lexer_scan_errors(
	const Source* restrict source,
	MemoryArea* restrict memArea);

#endif
