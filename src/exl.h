#ifndef EXL_H
#define EXL_H

#include <stdio.h>
#include <stdint.h>

#include "source.h"
#include "key_lock.h"
#include "lexer.h"
#include "parser.h"

typedef struct {
	struct {
		KeyLockCheckStage check_stage;
		const char* const* const keys;
		const uint16_t key_count;
		size_t key_index;
		const char* const* const* const locks;
		const size_t locks_count;
	} key_lock;
} Initializer;

#endif
