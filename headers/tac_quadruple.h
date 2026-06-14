#ifndef TAC_QUADRUPLE_H
#define TAC_QUADRUPLE_H

#include "allocator.h"
#include "parser.h"

typedef enum: uint64_t {
#define QUADRUPLE_TYPE(type) QuadrupleItemType_ ## type
	QUADRUPLE_TYPE(NO) = 0,
#undef QUADRUPLE_TYPE
} QuadrupleItemType;

typedef struct {
	QuadrupleItemType type;
	uint32_t misc;
} QuadrupleItem;

typedef struct {
	QuadrupleItem op;
	QuadrupleItem src1;
	QuadrupleItem src2;
	QuadrupleItem dst;
} QuadrupleEntry;

typedef struct {
	MemoryArea quadruples;
} QuadrupleList;

bool create_quadruples(
	Parser* parser,
	QuadrupleList tac_quadruple);

#endif
