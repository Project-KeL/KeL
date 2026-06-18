#ifndef TAC_QUADRUPLE_H
#define TAC_QUADRUPLE_H

#include "allocator.h"
#include "parser.h"

typedef enum: uint64_t {
#define QUADRUPLE_TYPE(type) QuadrupleItemType_ ## type
	QUADRUPLE_TYPE(NO) = 0,
//
	QUADRUPLE_TYPE(LIT),
// temporarie
	QUADRUPLE_TYPE(TEMP),
// operator
	QUADRUPLE_TYPE(ADD),
	QUADRUPLE_TYPE(SUB),
	QUADRUPLE_TYPE(MUL),
	QUADRUPLE_TYPE(DIV),
// key
	QUADRUPLE_TYPE(KEY),
	QUADRUPLE_TYPE(ASSIGN),
// PAL
	QUADRUPLE_TYPE(ARG),
	QUADRUPLE_TYPE(CALL),
	QUADRUPLE_TYPE(PAL),
	QUADRUPLE_TYPE(COUNT),
#undef QUADRUPLE_TYPE
} QuadrupleItemType;

typedef struct {
	QuadrupleItemType type;
	size_t offset_node;
} QuadrupleItem;

typedef struct {
	QuadrupleItem op;
	QuadrupleItem src1;
	QuadrupleItem src2;
	QuadrupleItem dst;
} QuadrupleEntry;

typedef struct {
	MemoryStack quadruples; // never popping stack
} QuadrupleList;

void initialize_quadruple_list(QuadrupleList* quadruple_list);
bool create_quadruple_list(
	Parser* parser,
	QuadrupleList* quadruple_list);
void destroy_quadruple_list(QuadrupleList* tac_quadruples);
void quadruple_list_append(
	QuadrupleEntry* entry,
	QuadrupleList* quadruple_list);

#endif
