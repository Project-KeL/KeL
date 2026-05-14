#include <assert.h>
#include "tac.h"

void initialize_TAC(TAC* tac) {
	assert(tac != NULL);
/*
	tac->lexer = NULL;
	initialize_memory_chain(&tac->quadruples);
*/
}

void destroy_TAC(TAC* tac) {
	if(tac == NULL)
		return;
/*
	MemoryChainIterator iterator;
	start_memory_chain_iterator(
		&tac->?,
		&iterator);
	end_memory_chain_iterator(&iterator);
	initialize_TAC(tac);
*/
}
