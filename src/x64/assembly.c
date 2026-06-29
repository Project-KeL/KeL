#include "assembly.h"
#include "register.h"
#include "tac_quadruple.h"
#include "x64_mapping.h"

static void create_tab(size_t count_tab) {
	for(
		size_t i = 0;
		i < count_tab;
		i += 1) {
			printf("\t");
	}
}

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
size_t count_tab,
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
	printf("\n");
	create_tab(count_tab);
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

static void create_call(
size_t i,
size_t count_tab,
const QuadEntry* entry,
Assembly* assembly
) {
	const TAC* tac = assembly->tac;
	const RegMap* regmap = assembly->regmap;
	static Reg saved_caller[] = {
		Reg_RDI,
		Reg_RSI,
		Reg_RDX,
		Reg_RCX,
		Reg_R8,
		Reg_R9,
		Reg_R10,
		Reg_R11,
		Reg_RAX};
	constexpr size_t count = sizeof(saved_caller) / sizeof(Reg);
	Reg reg_pushed[count] = {};
	size_t count_reg_pushed = 0;
	// caller-saved
	for(
	size_t j = 0;
	j < count;
	j += 1) {
		if(regmap_caller_saved(
			i,
			saved_caller[j],
			tac,
			regmap)
		== true) {
			printf(
				"push %s\n",
				regmap_to_str(saved_caller[j]));
			create_tab(count_tab);
			reg_pushed[count_reg_pushed] = saved_caller[j];
			count_reg_pushed += 1;
		}
	}
	// record the arguments
	size_t arity = entry->src2.offset_node;
	const QuadEntry* base_entry = tac->quadlist.quadruples.area.base;

	for(
	size_t j = i - arity;
	j != i;
	j += 1) {
		size_t k = j - (i - arity);
		const QuadEntry* entry_arg = base_entry + j;
		printf(
			"mov %s, ",
			regmap_to_str(saved_caller[k]));
		create_operand_right(
			&entry_arg->src1,
			assembly);
		printf("\n");
		create_tab(count_tab);
	}
	// emit the call
	const Token* tokens = tac->stab.parser->lexer->tokens.base;
	const Node* nodes = tac->stab.parser->nodes.base;
	size_t src1_offset_node = entry->src1.offset_node;
	const Token* token_PAL = tokens + nodes[src1_offset_node].offset_token;
	printf(
		"call %.*s\n",
		(int)(token_PAL->end - token_PAL->start),
		tac->stab.parser->lexer->source->content + token_PAL->start);
	// get the return value
	size_t dst_offset_node = entry->dst.offset_node;
	Reg reg_dst = regmap_from_slot_to_physical(
		(assembly->regmap->regslots.lifetimes + dst_offset_node)->slot.slot);

	if(reg_dst != Reg_RAX) {
		create_tab(count_tab);
		printf(
			"mov %s, rax\n",
			regmap_to_str(reg_dst));
	}
	// restore caller-saved
	for(
	size_t j = count_reg_pushed;
	j > 0;
	j -= 1) {
		create_tab(count_tab);
		printf(
			"pop %s\n",
			regmap_to_str(reg_pushed[j - 1]));
	}
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
	const char* code = assembly->tac->stab.parser->lexer->source->content;
	const Parser* parser = assembly->tac->stab.parser;
	const Node* nodes = parser->nodes.base;
	const Token* tokens = parser->lexer->tokens.base;
	const QuadList* quadlist = &assembly->tac->quadlist;
	size_t count_tab = 0;

	for(
	size_t i = 1;
	i < quadlist->quadruples.area.count - 1;
	i += 1) {
		const QuadEntry* entry = (QuadEntry*) quadlist->quadruples.area.base + i;

		if(entry->op.type == QuadItemType_ARG)
			continue; // create call already process the arguments (to avoid extra tab)

		size_t src1_offset_node = entry->src1.offset_node;
		size_t src2_offset_node = entry->src2.offset_node;
		size_t dst_offset_node = entry->dst.offset_node;
		const Token* token_src1 = tokens + nodes[src1_offset_node].offset_token;
		const Token* token_src2 = tokens + nodes[src2_offset_node].offset_token;
		const Token* token_dst = tokens + nodes[dst_offset_node].offset_token;
		SlotLifetime* lifetime_src1 = (SlotLifetime*) assembly->regmap->regslots.lifetimes + src1_offset_node;
		SlotLifetime* lifetime_src2 = (SlotLifetime*) assembly->regmap->regslots.lifetimes + src2_offset_node;
		SlotLifetime* lifetime_dst = (SlotLifetime*) assembly->regmap->regslots.lifetimes + dst_offset_node;
		Slot* slot_src1 = &lifetime_src1->slot;
		Slot* slot_src2 = &lifetime_src2->slot;
		Slot* slot_dst = &lifetime_dst->slot;

		Reg reg_src1 = regmap_from_slot_to_physical(slot_src1->slot);
		Reg reg_src2 = regmap_from_slot_to_physical(slot_src2->slot);
		Reg reg_dst = regmap_from_slot_to_physical(slot_dst->slot);


		if(entry->op.type == QuadItemType_SCOPE_END
		|| entry->op.type == QuadItemType_SCOPE_END_LAB
		|| entry->op.type == QuadItemType_SCOPE_END_PAL)
			count_tab -= 1;

		for(
		size_t j = 0;
		j < count_tab;
		j += 1) {
			printf("\t");
		}

		if(entry->op.type == QuadItemType_SCOPE
		|| entry->op.type == QuadItemType_SCOPE_LAB
		|| entry->op.type == QuadItemType_SCOPE_PAL)
			count_tab += 1;

		switch(entry->op.type) {
		case QuadItemType_SCOPE:
			printf("scope\n");
			break;
		case QuadItemType_SCOPE_END:
			printf(".\n");
			break;
		case QuadItemType_SCOPE_LAB:
			printf("#%.*s scope\n",
				(int)(token_src1->end - token_src1->start),
				code + token_src1->start);
			break;
		case QuadItemType_SCOPE_END_LAB:
			printf(".\n");
			break;
		case QuadItemType_SCOPE_PAL:
			printf("@%.*s scope\n",
				(int)(token_src1->end - token_src1->start),
				code + token_src1->start);
			break;
		case QuadItemType_SCOPE_END_PAL:
			printf(".\n");
			break;
		case QuadItemType_MOVE:
			printf("mov ");
			create_operand_left(
				&entry->dst,
				assembly);
			printf(", ");
			create_operand_right(
				&entry->src1,
				assembly);
			printf("\n");
			break;
		case QuadItemType_ADD:
			create_operator_algebraic(
				"add",
				count_tab,
				entry,
				assembly);
			printf("\n");
			break;
		case QuadItemType_SUB:
			create_operator_algebraic(
				"sub",
				count_tab,
				entry,
				assembly);
			printf("\n");
			break;
		case QuadItemType_CALL:
			create_call(
				i,
				count_tab,
				entry,
				assembly);
			break;
		default: break;
		}

	}

	return true;
}
