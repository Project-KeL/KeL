#include "assembly.h"
#include "register.h"
#include "tac_quadruple.h"
#include "x64_mapping.h"

static void create_operand_left(
const QuadItem* item,
Assembly* assembly) {
	switch(item->type) {
	case QuadItemType_KEY:
	case QuadItemType_TEMP:
		const Slot* slot = &(assembly->regmap->regslots.lifetimes + item->offset_node)->slot;
		printf(
			"%s",
			regmap_to_str(
				regmap_from_slot_to_physical(slot->slot)));
			break;
	default: break;
	}
}

static void create_operand_right(
const QuadItem* item,
Assembly* assembly) {
	const char* code = assembly->tac->stab.parser->lexer->source->content;
	const Parser* parser = assembly->tac->stab.parser;

	switch(item->type) {
	case QuadItemType_LIT:
		const Node* nodes = parser->nodes.base;
		const Token* tokens = parser->lexer->tokens.base;
		const Token* token = tokens + nodes[item->offset_node].offset_token;
		printf(
			"%.*s",
			(int)(token->end - token->start),
			code + token->start);
		break;
	case QuadItemType_KEY:
	case QuadItemType_TEMP:
		const Slot* slot = &(assembly->regmap->regslots.lifetimes + item->offset_node)->slot;
		printf(
			"%s",
			regmap_to_str(
				regmap_from_slot_to_physical(slot->slot)));
			break;
	default: break;
	}
}

static void create_operator_algebraic(
const char* op,
const QuadEntry* entry,
Assembly* assembly) {
	printf("mov ");
	create_operand_left(
		&entry->dst,
		assembly);
	printf(", ");
	create_operand_right(
		&entry->src1,
		assembly);
	printf("\n\t");
	printf(
		"%s ",
		op);
	create_operand_left(
		&entry->dst,
		assembly);
	printf(", ");
	create_operand_right(
		&entry->src2,
		assembly);
}

void initialize_assembly(Assembly* assembly) {
	assembly->path = NULL;
	assembly->file = NULL;
}

bool create_assembly(
const char* restrict path,
const TAC* tac,
const RegMap* regmap,
Assembly* restrict assembly) {
	assembly->file = fopen(
		path,
		"w");

	if(assembly->file == NULL) {
		printf("Cannot create the binary file.");
		destroy_assembly(assembly);
		return false;
	}

	assembly->path = path;
	assembly->tac = tac;
	assembly->regmap = regmap;
	return true;
}

bool destroy_assembly(Assembly* assembly) {
	if(assembly->file != NULL
	&& fclose(assembly->file) == EOF) {
		printf("Cannot close the binary.");
		return false;
	}

	assembly->path = NULL;
	assembly->file = NULL;
	return true;
}

bool assembly_file_write(Assembly* assembly) {
	const QuadList* quadlist = &assembly->tac->quadlist;

	for(
	size_t i = 1;
	i < quadlist->quadruples.area.count - 1;
	i += 1) {
		const QuadEntry* entry = (QuadEntry*) quadlist->quadruples.area.base + i;
		size_t src1_offset_node = entry->src1.offset_node;
		size_t src2_offset_node = entry->src2.offset_node;
		size_t dst_offset_node = entry->dst.offset_node;
		SlotLifetime* lifetime_src1 = (SlotLifetime*) assembly->regmap->regslots.lifetimes + src1_offset_node;
		SlotLifetime* lifetime_src2 = (SlotLifetime*) assembly->regmap->regslots.lifetimes + src2_offset_node;
		SlotLifetime* lifetime_dst = (SlotLifetime*) assembly->regmap->regslots.lifetimes + dst_offset_node;
		Slot* slot_src1 = &lifetime_src1->slot;
		Slot* slot_src2 = &lifetime_src2->slot;
		Slot* slot_dst = &lifetime_dst->slot;

		Reg reg_src1 = regmap_from_slot_to_physical(slot_src1->slot);
		Reg reg_src2 = regmap_from_slot_to_physical(slot_src2->slot);
		Reg reg_dst = regmap_from_slot_to_physical(slot_dst->slot);

		printf("\t");

		switch(entry->op.type) {
		case QuadItemType_MOVE:
			printf("mov ");
			create_operand_left(
				&entry->dst,
				assembly);
			printf(", ");
			create_operand_right(
				&entry->src1,
				assembly);
			break;
		case QuadItemType_ADD:
			create_operator_algebraic(
				"add",
				entry,
				assembly);
			break;
		case QuadItemType_SUB:
			create_operator_algebraic(
				"sub",
				entry,
				assembly);
			break;	
		default: break;
		}

		printf("\n");
	}

	return true;
}
