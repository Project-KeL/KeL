#include <assert.h>
#include "lexer.h"
#include "parser_allocator.h"
#include "parser_type.h"
#include "parser_utils.h"

/*
static NodeSubtypeChildTypeModifier operator_modifier_to_subtype_left(TokenSubtype subtype_token) {
	switch(subtype_token) {
	case TokenSubtype_AMPERSAND: return NodeSubtypeChildTypeModifier_AMPERSAND_LEFT;
	// brackets are always at the left side of the lock
	case TokenSubtype_LBRACKET: return NodeSubtypeChildTypeModifier_ARRAY;
	case TokenSubtype_RBRACKET: return NodeSubtypeChildTypeModifier_ARRAY;
	case TokenSubtype_MINUS: return NodeSubtypeChildTypeModifier_MINUS_LEFT;
	case TokenSubtype_PIPE: return NodeSubtypeChildTypeModifier_PIPE_LEFT;
	case TokenSubtype_PLUS: return NodeSubtypeChildTypeModifier_PLUS_LEFT;
	default: assert(false);
	}
}

static NodeSubtypeChildTypeModifier operator_modifier_to_subtype_right(TokenSubtype subtype_token) {
	switch(subtype_token) {
	case TokenSubtype_AMPERSAND: return NodeSubtypeChildTypeModifier_AMPERSAND_RIGHT;
	case TokenSubtype_MINUS: return NodeSubtypeChildTypeModifier_MINUS_RIGHT;
	case TokenSubtype_PIPE: return NodeSubtypeChildTypeModifier_PIPE_RIGHT;
	case TokenSubtype_PLUS: return NodeSubtypeChildTypeModifier_PLUS_RIGHT;
	default: assert(false);
	}
}
*/

static bool type_bind_child_token(
NodeTypeChildType type,
NodeSubtype subtype,
const Token* token,
Parser* parser) {
	if(!parser_allocator(parser))
		return false;

	*((Node*) parser->nodes.top) = (Node) {
		.is_child = true,
		.type = type,
		.subtype = subtype,
		.token = token,
		.child1 = NULL,
		.child2 = NULL};
	((Node*) parser->nodes.previous)->child1 = (Node*) parser->nodes.top;
	return true;
}
/*
static bool parse_type_lock_left(
size_t* i,
Parser* parser) {
	size_t buffer_i = *i;
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;
	// L side before lock
	while(tokens[buffer_i].type == TokenType_L
	   && parser_is_operator_modifier(tokens + buffer_i)) {
			// do not support arrays yet
			// arrays may contain expression so this case will need to be processed
			// `.child2` will hold this expression
			type_bind_child_token(
				NodeTypeChildType_MODIFIER,
				(NodeSubtype) operator_modifier_to_subtype_left(tokens[buffer_i].subtype),
				tokens + buffer_i,
				parser);
			// just ignore right brackets (for the moment)
			if(((Node*) parser->nodes.previous)->subtype == NodeSubtypeChildTypeModifier_ARRAY)
				buffer_i += 1;

			buffer_i += 1;
	}
	// R side before lock
	while(tokens[buffer_i].type == TokenType_R
	   && parser_is_operator_modifier(tokens + buffer_i)) {
		type_bind_child_token(
			NodeTypeChildType_MODIFIER,
			(NodeSubtype) operator_modifier_to_subtype_right(tokens[buffer_i].subtype),
			tokens + buffer_i,
			parser);

		if(((Node*) parser->nodes.previous)->subtype == NodeSubtypeChildTypeModifier_ARRAY)
			buffer_i += 1;

		buffer_i += 1;
	}

	if(tokens[buffer_i].type != TokenType_R)
		return false;

	*i = buffer_i;
	return true;
}

static bool parse_type_lock_right(
size_t* i,
Parser* parser) {
	size_t buffer_i = *i;
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;

	while(tokens[buffer_i].type == TokenType_R
	   && parser_is_operator_leveling(tokens + buffer_i)) {
		type_bind_child_token(
			NodeTypeChildType_MODIFIER,
			(NodeSubtype) operator_modifier_to_subtype_right(tokens[buffer_i].subtype),
			tokens + buffer_i,
			parser);
		buffer_i += 1;
	}

	if(parser_is_operator_modifier(tokens + buffer_i))
		return false;

	*i = buffer_i;
	return true;
}
*/
int if_type_create_nodes(
size_t* i,
MemoryArea* restrict memArea,
NodeSubtypeIdentificationBitScoped* restrict bit_scoped,
Parser* parser) {
	assert(i != NULL);
	assert(memArea != NULL);
	assert(bit_scoped != NULL);
	assert(parser != NULL);

	size_t buffer_i = *i;
	char* const memory = memArea->addr;
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;
	*bit_scoped = NodeSubtypeIdentificationBitScoped_NO;
	// if the current scope has at least one parameter memArea->addr[count_parenthesis_nest] is set to 1
	size_t count_parenthesis_nest = 0;

	if(parser_is_R_left_parenthesis(tokens + buffer_i))
		goto R_LPARENTHESIS_SKIP_PARAMETER;

	do {
		Node* lock;
TYPE:
		// lock alone
		
		/*
		if(parse_type_lock_left(
			&buffer_i,
			parser)
		== false)
			return false;
		*/
		if(tokens[buffer_i].subtype != TokenSubtype_LPARENTHESIS) {
			if(type_bind_child_token(
				NodeTypeChildType_LOCK,
				(NodeSubtype) NodeSubtypeChild_NO,
				tokens + buffer_i,
				parser)
			== false)
				return -1;

			size_t i_lock = buffer_i;
			lock = (Node*) parser->nodes.top;
			buffer_i += 1;
			memory[count_parenthesis_nest] = 1;
			/*
			if(parse_type_lock_right(
				&buffer_i,
				parser)
			== false)
				NodeSubtypeIdentificationBitScoped_INVALID;
			*/
			if(tokens[buffer_i].subtype != TokenSubtype_LPARENTHESIS
			&& count_parenthesis_nest == 0) {
				if(parser_is_scope_R(tokens + i_lock))
					*bit_scoped = NodeSubtypeIdentificationBitScoped_LABEL;

				break;
			}
		}
		// lock not alone (good luck)
		*bit_scoped = NodeSubtypeIdentificationBitScoped_LABEL_PARAMETERIZED;

		while(parser_is_R_left_parenthesis(tokens + buffer_i)) {
R_LPARENTHESIS:
			if(count_parenthesis_nest > 1
			// handle R left parenthesis at the first nesting level like in :(a :())
			|| (count_parenthesis_nest == 1
			 && !parser_is_parenthesis(tokens + buffer_i - 1)))
				goto R_LPARENTHESIS_SKIP_PARAMETER;

			if(!parser_is_key(tokens + buffer_i))
				return false;

			if(type_bind_child_token(
				NodeTypeChildType_LOCK,
				(NodeSubtype) NodeSubtypeChildTypeScoped_PARAMETER,
				tokens + buffer_i,
				parser)
			== false)
				return -1;

			buffer_i += 1;
R_LPARENTHESIS_SKIP_PARAMETER:
			if(type_bind_child_token(
				NodeTypeChildType_LOCK,
				(NodeSubtype) NodeSubtypeChildTypeScoped_RETURN_NONE,
				NULL,
				parser)
			== false)
				return -1;

			lock = (Node*) parser->nodes.top;
			buffer_i += 1;
			memory[count_parenthesis_nest] = 1;
			count_parenthesis_nest += 1;
			memory[count_parenthesis_nest] = 0;
		}

		if(tokens[buffer_i].subtype == TokenSubtype_LPARENTHESIS) {
			lock->subtype = NodeSubtypeChildTypeScoped_RETURN_TYPE;
// LPARENTHESIS:
			// handle nested empty parenthesis like in :(())
			if(tokens[buffer_i - 1].subtype == TokenSubtype_LPARENTHESIS)
				return 0;

			buffer_i += 1;
			count_parenthesis_nest += 1;
			memory[count_parenthesis_nest] = 0;

			if(parser_is_R_left_parenthesis(tokens + buffer_i))
				goto R_LPARENTHESIS;

			goto READ_PARAMETER;
		} else if(tokens[buffer_i].subtype == TokenSubtype_COMMA) {
COMMA:
			buffer_i += 1;
READ_PARAMETER:
			if(tokens[buffer_i - 1].subtype == TokenSubtype_LPARENTHESIS
			&& tokens[buffer_i].subtype == TokenSubtype_RPARENTHESIS)
				goto RPARENTHESIS;

			memory[count_parenthesis_nest] = 1;

			if(parser_is_key(tokens + buffer_i)) {
				// ignore parameter after the first nesting level
				if(count_parenthesis_nest > 1) {
					buffer_i += 1;
					goto TYPE;
				}

				if(type_bind_child_token(
					NodeTypeChildType_LOCK,
					(NodeSubtype) NodeSubtypeChildTypeScoped_PARAMETER,
					tokens + buffer_i,
					parser)
				== false)
					return -1;

				buffer_i += 1;
				goto TYPE;
			} else if(count_parenthesis_nest > 1
			       && parser_is_lock(tokens + buffer_i)) {
				goto TYPE;
			} else {
				// a lock must succeed a key at the first nesting level
				return 0;
			}
		} else if(tokens[buffer_i].subtype == TokenSubtype_RPARENTHESIS) {
RPARENTHESIS:
			if(memory[count_parenthesis_nest] == 0) {
				if(type_bind_child_token(
					NodeTypeChildType_LOCK,
					(NodeSubtype) NodeSubtypeChildTypeScoped_PARAMETER_NONE,
					NULL,
					parser)
				== false)
					return -1;
			}

			do {
				buffer_i += 1;
				count_parenthesis_nest -= 1;
			} while(tokens[buffer_i].subtype == TokenSubtype_RPARENTHESIS);

			if(tokens[buffer_i].subtype == TokenSubtype_COMMA)
				goto COMMA;
		} else if(parser_is_key(tokens + buffer_i)) {
				// keep ignoring parameter after the first nesting level
				if(count_parenthesis_nest > 1) {
					buffer_i += 1;
					goto TYPE;
				}
				// a parameter after an R left parenthesis like in :(a :b)
				if(type_bind_child_token(
					NodeTypeChildType_LOCK,
					(NodeSubtype) NodeSubtypeChildTypeScoped_PARAMETER,
					tokens + buffer_i,
					parser)
				== false)
					return -1;

				buffer_i += 1;
				goto TYPE;
		} else if(parser_is_lock(tokens + buffer_i)) {
			goto TYPE;
		} else {
			return 0;
		}
	} while(count_parenthesis_nest != 0);

	*i = buffer_i;
	return 1;
}
