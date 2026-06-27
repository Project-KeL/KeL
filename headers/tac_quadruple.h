#ifndef TAC_QUADRUPLE_H
#define TAC_QUADRUPLE_H

#include "allocator.h"
#include "parser.h"

typedef enum: uint64_t {
#define QUAD_TYPE(type) QuadItemType_ ## type
	QUAD_TYPE(NO) = 0,
// scop
	QUAD_TYPE(SCOPE),
	QUAD_TYPE(SCOPE_END),
	QUAD_TYPE(SCOPE_LAB),
	QUAD_TYPE(SCOPE_END_LAB),
	QUAD_TYPE(SCOPE_PAL),
	QUAD_TYPE(SCOPE_END_PAL),
//
	QUAD_TYPE(LIT),
// temporarie
	QUAD_TYPE(TEMP),
// operator
	QUAD_TYPE(MOVE),
	QUAD_TYPE(ADD),
	QUAD_TYPE(SUB),
	QUAD_TYPE(MUL),
	QUAD_TYPE(DIV),
// key
	QUAD_TYPE(KEY),
// PAL
	QUAD_TYPE(ARG),
	QUAD_TYPE(CALL),
	QUAD_TYPE(PAL),
	QUAD_TYPE(COUNT),
#undef QUAD_TYPE
} QuadItemType;

typedef struct {
	QuadItemType type;
	size_t offset_node;
} QuadItem;

typedef struct {
	QuadItem op;
	QuadItem src1;
	QuadItem src2;
	QuadItem dst;
} QuadEntry;

typedef struct {
	MemoryStack quadruples; // never popping stack
} QuadList;

QuadItem create_quaditem_null(void);
bool quadlist_allocator_shrink_append_null(QuadList* quadlist);
void initialize_quadlist(QuadList* quadlist);
bool create_quadlist(
	Parser* parser,
	QuadList* quadlist);
void destroy_quadlist(QuadList* tac_quadruples);
void quadlist_append(
	QuadEntry* entry,
	QuadList* quadlist);

#endif
