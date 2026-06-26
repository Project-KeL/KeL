#include "assembly.h"
#include "register.h"
#include "tac_quadruple.h"
#include "x64_mapping.h"

void initialize_assembly(Assembly* assembly) {
	assembly->path = NULL;
	assembly->file = NULL;
}

bool create_assembly(
const char* restrict path,
const QuadList* quadlist,
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
	assembly->quadlist = quadlist;
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
	for(
	size_t i = 1;
	i < assembly->quadlist->quadruples.area.count - 1;
	i += 1) {
		const QuadEntry* entry = (QuadEntry*) assembly->quadlist->quadruples.area.base + i;
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

		switch(entry->op.type) {
		case QuadItemType_MOVE:
		default: break;
		}
	}

	return true;
}
