#include <assert.h>
#include <stddef.h>
#include "lexer_def.h"
#include "parser_allocator.h"
#include "parser_introduction.h"
#include "parser_type.h"
#include "parser_utils.h"
#include <stdio.h>

// `child1` points to the type and `child2` to the scope if there is an initialization
 
NodeSubtypeIntroductionBitCommand parser_identifier_token_subtype_TO_node_subtype_introduction_bit_command(TokenSubtype subtype_token) {
	switch(subtype_token) {
	case TokenSubtype_HASH: return NodeSubtypeIntroductionBitCommand_HASH;
	case TokenSubtype_AT: return NodeSubtypeIntroductionBitCommand_AT;
	case TokenSubtype_EXCLAMATION_MARK: return NodeSubtypeIntroductionBitCommand_EXCLAMATION_MARK;
	default: assert(false);
	}
}

static NodeSubtypeLiteral token_subtype_literal_to_subtype(TokenSubtype subtype_token) {
	return (NodeSubtypeLiteral) subtype_token;
}

static int if_declaration_create_nodes(
size_t* i,
MemoryArea* restrict memArea,
MemoryChainLink** link_introduction,
Node** node_introduction,
Parser* parser) {
	assert(i != NULL);
	assert(node_introduction != NULL);
	assert(parser != NULL);

	size_t buffer_i = *i;
	size_t i_qualifier = buffer_i;
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;
	NodeSubtype subtype = NodeSubtype_NO;
	MemoryChainState memChain_state;
	initialize_memory_chain_state(&memChain_state);

	while(parser_is_qualifier(tokens + buffer_i)) buffer_i += 1;

	if(tokens[buffer_i].type != TokenType_COMMAND)
		return 0;

	subtype |= parser_identifier_token_subtype_TO_node_subtype_introduction_bit_command(tokens[buffer_i].subtype);
	buffer_i += 1;

	if(tokens[buffer_i].subtype != TokenSubtype_IDENTIFIER)
		return 0;

	memory_chain_state_save(
		&parser->nodes,
		&memChain_state);

	if(!parser_allocator(parser))
		return -1;

	*((Node*) parser->nodes.top) = (Node) {
		.is_child = false,
		.type = NodeType_INTRODUCTION,
		.subtype = subtype,
		.token = tokens + buffer_i,
		.Introduction = {
			.type = NULL,
			.initialization = NULL}};
	buffer_i += 1;
	*node_introduction = (Node*) parser->nodes.top;
	*link_introduction = parser->nodes.last;
/*
	while(parser_is_qualifier(tokens + i_qualifier)) {
		if(!parser_allocator(parser))
			return -1;

		*((Node*) parser->nodes.top) = (Node) {
			.is_child = true,
			.type = NodeType_QUALIFIER,
			.subtype = tokens[i_qualifier].subtype,
			.token = tokens + i_qualifier,
			.Qualifier = {.next = NULL}};
		// case identifier: .type points to the beginning of the type
		// case qualifier: .next points to the next qualifiers
		// so `.child` is ok
		((Node*) parser->nodes.previous)->child = (Node*) parser->nodes.top;
		i_qualifier += 1;
	}
*/
	// type deduction later
	NodeSubtypeIntroductionBitScoped bit_scoped;
	switch(if_type_create_nodes(
		&buffer_i,
		memArea,
		&bit_scoped,
		parser)) {
	case -1: return -1;
	case 0:
		memory_chain_state_restore(
			&parser->nodes,
			&memChain_state);
		return 0;
	case 1:
		(*node_introduction)->subtype |= bit_scoped;
		break;
	}
	
	*i = buffer_i;
	return 1;
}

static int if_initialization_create_node(
bool nodes_initialization,
size_t* i,
Node* node_introduction,
Parser* parser) {
	// just parse literals for the moment, expressions later
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;
	size_t buffer_i = *i;

	if(parser_is_scope_L(tokens + buffer_i))
		return 1; // `Introduction.initialization` is set in `parser.c`

	return 0; // remove later
/*
	MemoryChainState memChain_state;
	initialize_memory_chain_state(&memChain_state);
	memory_chain_state_save(
		&parser->nodes,
		&memChain_state);

	if(nodes_initialization
	&& !parser_allocator(parser))
		return -1;

	if(tokens[buffer_i].type == TokenType_LITERAL) {
		if(!nodes_initialization)
			goto NODES_NO;

		*((Node*) parser->nodes.top) = (Node) {
			.type = NodeType_LITERAL,
			.subtype = token_subtype_literal_to_subtype(tokens[buffer_i].subtype),
			.token = tokens + buffer_i,
			.child = NULL};
		buffer_i += 1;
	} else {
		memory_chain_state_restore(
			&parser->nodes,
			&memChain_state);
		return 0;
	}

	node_identifier->Identifier.initialization = (Node*) parser->nodes.top;
NODES_NO:
	*i = buffer_i;
	return 1;
*/
}

int if_introduction_create_nodes(
bool nodes_initialization,
size_t* i,
MemoryArea* restrict memArea,
MemoryChainLink** link_introduction,
Node** node_introduction,
Parser* parser) {
	assert(i != NULL);
	assert(memArea != NULL);
	assert(node_introduction != NULL);
	assert(parser != NULL);

	size_t buffer_i = *i;

	switch(if_declaration_create_nodes(
		&buffer_i,
		memArea,
		link_introduction,
		node_introduction,
		parser)) {
	case -1: return -1;
	case 0: return 0;
	}
	// add the type as child nodes in `.child1`	
	switch(if_initialization_create_node(
		nodes_initialization,
		&buffer_i,
		*node_introduction,
		parser)) {
	case -1: return -1;
	case 0: // declaration case
		(*node_introduction)->subtype |= NodeSubtypeIntroductionBitType_DECLARATION;
		break;
	case 1: // initialization case
		(*node_introduction)->subtype |= NodeSubtypeIntroductionBitType_INITIALIZATION;
		break;
	}
	// no null token because a type may appear between nodes
#ifndef NDEBUG
	parser_is_valid_introduction(*node_introduction);
#endif
	*i = buffer_i;
	return 1;
}

bool parser_is_introduction(const Node* node) {
	return node->is_child == false
        && node->type == NodeType_INTRODUCTION;
}

bool parser_is_valid_introduction(const Node* node) {
	assert(parser_is_introduction(node));

	return true;
}

bool parser_introduction_is_declaration(const Node* node) {
	assert(node != NULL);

	return node->is_child
	    && (node->subtype & MASK_BIT_NODE_SUBTYPE_INTRODUCTION_TYPE)
	    == NodeSubtypeIntroductionBitType_DECLARATION;
}

bool parser_introduction_is_initialization(const Node* node) {
	assert(node != NULL);

	return node->is_child
	    && (node->subtype & MASK_BIT_NODE_SUBTYPE_INTRODUCTION_TYPE)
	    == NodeSubtypeIntroductionBitType_INITIALIZATION;
}

bool parser_introduction_is_label(const Node* node) {
	assert(node != NULL);

	return node->is_child
	    && (node->subtype & MASK_BIT_NODE_SUBTYPE_INTRODUCTION_SCOPED)
	    == NodeSubtypeIntroductionBitScoped_LABEL;
}

bool parser_introduction_is_PAL(const Node* node) {
	assert(node != NULL);

	return (node->subtype & MASK_BIT_NODE_SUBTYPE_INTRODUCTION_SCOPED)
	    == NodeSubtypeIntroductionBitScoped_PAL;
}

void parser_introduction_set_type(
Node* node,
Node* type) {
#ifndef NDEBUG
	parser_is_valid_introduction(node);
	parser_is_valid_type(type);
#endif
	node->Introduction.type = type;
}

void parser_introduction_set_initialization(
Node* node,
Node* initialization) {
#ifndef NDEBUG
	parser_is_valid_introduction(node);
#endif
	node->Introduction.initialization = initialization;
}

const Node* parser_introduction_get_type(const Node* node) {
#ifndef NDEBUG
	parser_is_valid_introduction(node);
#endif
	return node->Introduction.type;
}

const Node* parser_introduction_get_initialization(const Node* node) {
#ifndef NDEBUG
	parser_is_valid_introduction(node);
#endif
	return node->Introduction.initialization;
}
