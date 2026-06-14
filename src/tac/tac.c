#include <assert.h>
#include "parser.h"
#include "stab.h"
#include "tac.h"
#include <stdio.h>
void initialize_tac(TAC* tac) {
	assert(tac != NULL);

	initialize_stab(&tac->stab);
}

bool create_tac(
Parser* parser,
TAC* tac) {
	if(create_stab(
		parser,
		&tac->stab)
	== false)
		return false;

	bool error = false;
	// SCOPE_0
	stab_push_scope(&tac->stab);
	const Node* nodes = parser->nodes.base;
	const size_t count = parser->nodes.count;
	// sentinels only
	if(count <= 2)
		return false;
	// traverse nodes
	// substree = root + descendant nodes
	size_t* start_subtree = calloc(
		count,
		sizeof(size_t));
	size_t* stack_index = malloc(count * sizeof(size_t));
	size_t* stack_depth = malloc(count * sizeof(size_t));

	if(start_subtree == NULL
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

	const Token* tokens = parser->lexer->tokens.base;

	while(top != 0) {
		top -= 1;
		const size_t i = stack_index[top];
		const size_t depth = stack_depth[top];
		// record ID in the symbol table
		if(nodes[i].type == NodeType_ID) {
			stab_push_entry(
				tokens + nodes[i].offset_token,
				&tac->stab);
		// start a new frame in the symbol table
		} else if(nodes[i].type == NodeType_SCOPE) {
			stab_push_scope(&tac->stab);
		// pop the scope
		} else if(nodes[i].type == NodeType_SCOPE_END) {
			stab_pop_scope(&tac->stab);
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

	if(error)
		destroy_tac(tac);

	return !error;
}

void destroy_tac(TAC* tac) {
	if(tac == NULL)
		return;

	destroy_stab(&tac->stab);
	initialize_tac(tac);;
}
