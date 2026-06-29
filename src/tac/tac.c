#include <assert.h>
#include "allocator.h"
#include "parser.h"
#include "tac_stab.h"
#include "tac.h"
#include "tac_expression.h"
#include "tac_quadruple.h"
#include "tac_stab.h"
#include <stdio.h>
void initialize_tac(TAC* tac) {
	assert(tac != NULL);

	initialize_tac_stab(&tac->stab);
	initialize_quadlist(&tac->quadlist);
}

bool create_tac(
Parser* parser,
TAC* tac) {
	initialize_tac_stab(&tac->stab);
	initialize_quadlist(&tac->quadlist);

	if(!create_tac_stab(
		parser,
		&tac->stab)
	|| create_quadlist(
		parser,
		&tac->quadlist)
	== false)
		return false;

	bool error = false;
	// SCOPE_0
	tac_stab_push_scope(&tac->stab);
	const Node* nodes = parser->nodes.base;
	const size_t count = parser->nodes.count;
	// sentinels only
	if(count <= 2)
		return false;
	// traverse nodes
	// substree = root + descendant nodes
	MemoryStack stack_buffer;
	initialize_memory_stack(&stack_buffer);
	size_t* start_subtree = calloc(
		count,
		sizeof(size_t));
	size_t* stack_index = malloc(count * sizeof(size_t));
	size_t* stack_depth = malloc(count * sizeof(size_t));

	if(!create_memory_stack(
		parser->lexer->source->length,
		sizeof(Node*),
		&stack_buffer)
	|| start_subtree == NULL
	|| stack_depth == NULL
	|| stack_index == NULL) {
		error = true;
		goto END;
	}

	{
		// fill `stack_subtree`
		// a leaf is its own subtree
		size_t top = 0;

		for(size_t k = 1;
		k < count - 1;
		k += 1) {
			size_t start = k;
			// the last popped child is the one at the very left
			for(uint32_t c = 0;
			c < nodes[k].arity;
			c += 1) {
				top -= 1;
				start = stack_index[top]; // the first node has arity = 0
			}

			start_subtree[k] = start;
			stack_index[top] = start;
			top += 1;
		}
	}

	size_t top = 0;

	for(size_t end = count - 2
	;
	;) {
		stack_index[top] = end;
		stack_depth[top] = 0;
		top += 1;
		size_t start = start_subtree[end];

		if(start <= 1)
			break;

		end = start - 1;
	}

	while(top != 0) {
		top -= 1;
		size_t i = stack_index[top];
		const size_t depth = stack_depth[top];
		// record ID in the symbol table
		if(((nodes[i].type == NodeType_DECL_VAR
		  || nodes[i].type == NodeType_DECL_LAB
		  || nodes[i].type == NodeType_DECL_PAL)
		 && nodes[start_subtree[i]].type == NodeType_ID)
		|| nodes[i].type == NodeType_PARAM) {
			tac_stab_push_entry(
				start_subtree[i],
				&tac->stab);
			QuadItemType type = QuadItemType_NO;
			
			if(nodes[i].type == NodeType_DECL_LAB
			&& nodes[i - 1].type == NodeType_INIT_LAB) {
				type = QuadItemType_SCOPE_LAB;
			} else if(nodes[i].type == NodeType_DECL_PAL
			       && nodes[i - 1].type == NodeType_INIT_PAL)
				type = QuadItemType_SCOPE_PAL;

			if(type != QuadItemType_NO) {
				tac_stab_push_scope(&tac->stab);
				QuadEntry entry = (QuadEntry) {
				.op = (QuadItem) {
					.type = type,
					.offset_node = i},
				.src1 = (QuadItem) {
					.type = QuadItemType_KEY,
					.offset_node = start_subtree[i]},
				.src2 = create_quaditem_null(),
				.dst = create_quaditem_null()};
				quadlist_append(
					&entry,
					&tac->quadlist);
			}
		// start a new frame in the symbol table
		} else if(nodes[i].type == NodeType_SCOPE
			   && nodes[i + 1].type != NodeType_INIT_LAB
		       && nodes[i + 1].type != NodeType_INIT_PAL) {
			tac_stab_push_scope(&tac->stab);
			QuadEntry entry = (QuadEntry) {
				.op = (QuadItem) {
					.type = QuadItemType_SCOPE,
					.offset_node = i},
				.src1 = create_quaditem_null(),
				.src2 = create_quaditem_null(),
				.dst = create_quaditem_null()};
			quadlist_append(
				&entry,
				&tac->quadlist);
		// pop the scope
		} else if(nodes[i].type == NodeType_SCOPE_END) {
			tac_stab_pop_scope(&tac->stab);
			QuadEntry entry;

			if(nodes[i + 2].type == NodeType_INIT_LAB) { // i is well defined (SCOPE_END + sentinel at least)
				entry = (QuadEntry) {
					.op = (QuadItem) {
						.type = QuadItemType_SCOPE_END_LAB,
						.offset_node = i},
					.src1 = create_quaditem_null(),
					.src2 = create_quaditem_null(),
					.dst = create_quaditem_null()};
			} else if(nodes[i + 2].type == NodeType_INIT_PAL) {
				entry = (QuadEntry) {
					.op = (QuadItem) {
						.type = QuadItemType_SCOPE_END_PAL,
						.offset_node = i},
					.src1 = create_quaditem_null(),
					.src2 = create_quaditem_null(),
					.dst = create_quaditem_null()};
			} else {
				entry = (QuadEntry) {
					.op = (QuadItem) {
						.type = QuadItemType_SCOPE_END,
						.offset_node = i},
					.src1 = create_quaditem_null(),
					.src2 = create_quaditem_null(),
					.dst = create_quaditem_null()};
			}

			quadlist_append(
				&entry,
				&tac->quadlist);
		} else if(nodes[i].type == NodeType_EXP) {
			size_t dst = 0;

			if(i + 2 < count
			&& nodes[i + 1].type == NodeType_INIT_VAR)
				dst = start_subtree[i + 2];

			tac_create_expression(
				start_subtree[i],
				i,
				dst,
				&stack_buffer,
				tac);
			continue;
		}
		// push the child from right to left
		size_t end = i - 1;

		for(uint32_t c = 0;
		c < nodes[i].arity;
		c += 1) {
			stack_index[top] = end;
			stack_depth[top] = depth + 1;
			top += 1;
			const size_t start = start_subtree[end];

			if(start <= 1)
				break; // avoid underflow: the left most child is reached

			end = start - 1;
		}
	}

	if(!quadlist_allocator_shrink_append_null(&tac->quadlist))
		error = true;
END:
	free(stack_depth);
	free(stack_index);
	free(start_subtree);
	destroy_memory_stack(&stack_buffer);

	if(error)
		destroy_tac(tac);

	return !error;
}

void destroy_tac(TAC* tac) {
	if(tac == NULL)
		return;

	destroy_quadlist(&tac->quadlist);
	destroy_tac_stab(&tac->stab);
	initialize_tac(tac);
}
