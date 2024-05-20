#include <assert.h>
#include <string.h>
#include "parser.h"
#include "parser_error.h"
#include "parser_utils.h"
#include <stdio.h>

// i is the current token to be processed
// j is the current node to be created

static long int error = 0;

static bool allocate_chunk(
long int minimum,
Parser* restrict parser) {
#define NODES_CHUNK 4096
	if(parser->count <= minimum) {
		const long int reserve = minimum / NODES_CHUNK + 1;
		Node* nodes_realloc = realloc(
			parser->nodes,
			reserve * NODES_CHUNK * sizeof(Node));

		if(nodes_realloc == NULL)
			return false;

		parser->nodes = nodes_realloc;
		parser->count = reserve * NODES_CHUNK;
	}

	return true;
#undef NODES_CHUNK
}

static int set_error(long int value) {
	if(value == -1)
		return -1;

	error = value;
	return value;
}

static void create_child_key_type(
NodeSubtypeChildKeyType subtype,
long int array_bound,
long int i,
const Token* restrict token,
Node* node) {
	*node = (Node) {
		.type = NodeType_CHILD,
		.subtype = subtype,
		.value = array_bound,
		.child1 = NULL,
		.child2 = NULL};
}

static bool if_scope_create_node(
long int i,
long int j,
Parser* restrict parser) {
	if(parser_is_scope(
		i,
		parser->lexer)
	== false)
		return false;

	parser->nodes[j] = (Node) {
		.type = NodeType_SCOPE_START,
		.subtype = NodeSubtypeScope_NO};
	return true;
}

static int if_period_create_node(
long int i,
long int j,
Parser* parser) {
	if(parser->lexer->tokens[i].subtype != TokenSubtype_PERIOD)
		return false;

	long int j_scope_start = parser_get_j_scope_start_from_end(
			j,
			parser);
	parser->nodes[j] = (Node) {
		.type = NodeType_SCOPE_END,
		.subtype = parser->nodes[j_scope_start].subtype};
	parser->nodes[j_scope_start].child = &parser->nodes[j];
	return true;
}

static NodeSubtypeChildKeyType operator_modifiers_to_subtype_left(TokenSubtype token_subtype) {
	switch(token_subtype) {
	case TokenSubtype_AMPERSAND: return NodeSubtypeChildKeyType_AMPERSAND_LEFT;
	// brackets are always at the left side of the lock
	case TokenSubtype_LBRACKET: return NodeSubtypeChildKeyType_ARRAY;
	case TokenSubtype_RBRACKET: return NodeSubtypeChildKeyType_ARRAY;
	case TokenSubtype_MINUS: return NodeSubtypeChildKeyType_MINUS_LEFT;
	case TokenSubtype_PIPE: return NodeSubtypeChildKeyType_PIPE_LEFT;
	case TokenSubtype_PLUS: return NodeSubtypeChildKeyType_PLUS_LEFT;
	default: assert(false);
	}
}

static NodeSubtypeChildKeyType operator_modifiers_to_subtype_right(TokenSubtype token_subtype) {
	switch(token_subtype) {
	case TokenSubtype_AMPERSAND: return NodeSubtypeChildKeyType_AMPERSAND_RIGHT;
	case TokenSubtype_MINUS: return NodeSubtypeChildKeyType_MINUS_RIGHT;
	case TokenSubtype_PIPE: return NodeSubtypeChildKeyType_PIPE_RIGHT;
	case TokenSubtype_PLUS: return NodeSubtypeChildKeyType_PLUS_RIGHT;
	default: assert(false);
	}
}

static int if_type_create_nodes(
long int* i,
long int* j,
Node* parent,
Parser* parser) {
	const Token* tokens = parser->lexer->tokens;
	long int buffer_i = *i;
	long int buffer_j = *j;
	// allocate modifier parts of the type
	{
		long int count_tokens = 0;

		while(tokens[buffer_i + count_tokens].type == TokenType_L
		   && parser_is_operator_modifier(&tokens[buffer_i + count_tokens])) count_tokens += 1;

		while(tokens[buffer_i + count_tokens].type == TokenType_R
		   && parser_is_operator_modifier(&tokens[buffer_i + count_tokens])) count_tokens += 1;

		if(!parser_is_lock(&tokens[buffer_i + count_tokens]))
			return 0;

		count_tokens += 1;

		while(tokens[buffer_i + count_tokens].type == TokenType_R
		   && parser_is_operator_leveling(&tokens[buffer_i + count_tokens])) count_tokens += 1;

		if(allocate_chunk(
			buffer_j + count_tokens,
			parser)
		== false)
			return -1;
	}
	// left side
	while(!parser_is_lock(&tokens[buffer_i])) {
			// do not support arrays yet
			// arrays may contain expression so this case will need to be processed
			// `.child2` will hold this expression
			parser->nodes[buffer_j] = (Node) {
				.type = NodeType_CHILD,
				.subtype = operator_modifiers_to_subtype_left(tokens[buffer_i].subtype),
				.token = &tokens[buffer_i],
				.child1 = NULL,
				.child2 = NULL};
			// the type must be binded with another node (declaration, initialization, previous node, ...)
			parent->child1 = &parser->nodes[buffer_j];
			parent = &parser->nodes[buffer_j];
			// just ignore right brackets (for the moment)
			if(parent->subtype == NodeSubtypeChildKeyType_ARRAY)
				buffer_i += 1;

			buffer_i += 1;
			buffer_j += 1;
	}
	// lock
	parser->nodes[buffer_j] = (Node) {
		.type = NodeType_CHILD,
		.subtype = NodeSubtype_NO,
		.token = &tokens[buffer_i],
		.child1 = NULL,
		.child2 = NULL};
	parent->child1 = &parser->nodes[buffer_j];
	parent = &parser->nodes[buffer_j];
	buffer_i += 1;
	buffer_j += 1;
	// right side
	while(tokens[buffer_i].type == TokenType_R) {
		parser->nodes[buffer_j] = (Node) {
			.type = NodeType_CHILD,
			.subtype = operator_modifiers_to_subtype_right(tokens[buffer_i].subtype),
			.token = &tokens[buffer_i],
			.child1 = NULL,
			.child2 = NULL};
		parent->child1 = &parser->nodes[buffer_j];
		parent = &parser->nodes[buffer_j];
		buffer_i += 1;
		buffer_j += 1;
	}
	// we read the next token but 
	*i = buffer_i;
	*j = buffer_j - 1;
	return 1;
}

int if_declaration_create_nodes(
long int* i,
long int* j,
Parser* parser) {
	Token* tokens = parser->lexer->tokens;
	long int buffer_i = *i;
	long int buffer_j = *j;

	if(tokens[buffer_i].subtype != TokenSubtype_AT)
		return 0;

	buffer_i += 1;

	if(tokens[buffer_i].type != TokenType_IDENTIFIER)
		return 0;

	if(allocate_chunk(
		buffer_j + 1,
		parser)
	< 0)
		return -1;

	parser->nodes[buffer_j] = (Node) {
		.type = NodeType_DECLARATION,
		.subtype = NodeSubtype_NO,
		.token = &tokens[buffer_i]};
	buffer_i += 1;
	buffer_j += 1;

	switch(if_type_create_nodes(
		&buffer_i,
		&buffer_j,
		&parser->nodes[buffer_j - 1],
		parser)
	) {
	case -1: return -1;
	case 0: return 0;
	}

	*i = buffer_i;
	*j = buffer_j;
	return 1;
}

bool create_parser(
const Lexer* lexer,
Allocator* restrict allocator,
Parser* restrict parser) {
	assert(lexer != NULL);
	assert(lexer->tokens != NULL);
	parser->lexer = lexer;
	parser->nodes = NULL;
	parser->count = 0;
	const char* code = lexer->source->content;
	const Token* tokens = lexer->tokens;
	long int i = 0;
	long int j = 0;

	if(!parser_scan_errors(lexer))
		return false;

	while(i < lexer->count) {
		// allocation
		if(allocate_chunk(
			i,
			parser)
		== false) {
			destroy_parser(parser);
			return false;
		}
		// create nodes
		if(if_scope_create_node(
			i,
			j,
			parser)
		== true) {
			// OK
		} else if(set_error(
			if_declaration_create_nodes(
				&i,
				&j,
				parser))
		== 1) {
			// OK
		} else {
			// check end of scope (period) or end of instruction (semicolon)
			if(set_error(
				if_period_create_node(
					i,
					j,
					parser))
			== 1) {
				// OK
			} else if(tokens[i].subtype == TokenSubtype_SEMICOLON) {
				i += 1;
				continue; // no new node
			} else {
				destroy_parser(parser);
				return false;
			}
		}
		// error checking
		if(error == -1) {
			destroy_parser(parser);
			return false;
		}
		// loop end
		i += 1;
		j += 1;
	}

	if(j == 0) {
		destroy_parser(parser);
		return false;
	}

	parser->count = j;
	Node* nodes_realloc = realloc(
		parser->nodes,
		(parser->count + 1) * sizeof(Node));

	if(nodes_realloc == NULL) {
		destroy_parser(parser);
		return false;
	}

	parser->nodes = nodes_realloc;
	parser->nodes[parser->count] = (Node) {.type = NodeType_NO};
	return true;
}

void destroy_parser(Parser* restrict parser) {
	parser->lexer = NULL;
	free(parser->nodes);
	parser->nodes = NULL;
	parser->count = 0;
}
