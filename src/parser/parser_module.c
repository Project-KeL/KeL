#include "parser_module.h"
#include "parser_node.h"
#include "parser_utils.h"

bool if_MOD_create_operator(
size_t* i,
size_t* j,
MemoryStack* stack_operator,
Parser* parser) {
	const Token* tokens = parser->lexer->tokens.base;
	size_t buffer_i = *i;
	size_t buffer_j = *j;

	if(parser_is_IMOD(tokens + *i)) {
		Operator operator = (Operator) {
			.type = NodeType_GRP_IMOD,
			.precedence = 0,
			.count_arity = 0,
			.token = *i};
		memory_stack_push(
			(char*) &operator,
			stack_operator);
		buffer_i += 1;

		while(parser_is_key(tokens + buffer_i)) {
			parser_create_leaf(
				NodeType_IMOD,
				buffer_i,
				&buffer_j,
				stack_operator,
				parser);
			buffer_i += 1;
		}
	} else if(parser_is_OMOD(tokens + *i)) {
		Operator operator = (Operator) {
			.type = NodeType_GRP_OMOD,
			.precedence = 0,
			.count_arity = 0,
			.token = *i};
		memory_stack_push(
			(char*) &operator,
			stack_operator);
		buffer_i += 1;

		while(parser_is_key(tokens + buffer_i)) {
			parser_create_leaf(
				NodeType_OMOD,
				buffer_i,
				&buffer_j,
				stack_operator,
				parser);
			buffer_i += 1;
		}
	} else
		return false;

	*i = buffer_i;
	*j = buffer_j;
	return true;
}
