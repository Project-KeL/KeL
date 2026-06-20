#include <assert.h>
#include "allocator.h"
#include "parser.h"
#include "stab.h"
#include "tac.h"
#include "tac_expression.h"
#include "tac_quadruple.h"
#include <stdio.h>
void initialize_tac(TAC* tac) {
	assert(tac != NULL);

	initialize_tac_stab(&tac->stab);
	initialize_quadruple_list(&tac->quadruple_list);
}

bool create_tac(
Parser* parser,
TAC* tac) {
	initialize_tac_stab(&tac->stab);
	initialize_quadruple_list(&tac->quadruple_list);

	if(!create_tac_stab(
		parser,
		&tac->stab)
	|| create_quadruple_list(
		parser,
		&tac->quadruple_list)
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
		if((nodes[i].type == NodeType_DECL_VAR
		 || nodes[i].type == NodeType_DECL_PAL)
		&& nodes[start_subtree[i]].type == NodeType_ID) {
			tac_stab_push_entry(
				start_subtree[i],
				&tac->stab);
		// start a new frame in the symbol table
		} else if(nodes[i].type == NodeType_SCOPE) {
			tac_stab_push_scope(&tac->stab);
		// pop the scope
		} else if(nodes[i].type == NodeType_SCOPE_END) {
			tac_stab_pop_scope(&tac->stab);
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

	destroy_quadruple_list(&tac->quadruple_list);
	destroy_tac_stab(&tac->stab);
	initialize_tac(tac);
}
