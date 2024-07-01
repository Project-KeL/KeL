#include <assert.h>
#include <stddef.h>
#include "lexer_def.h"
#include "parser_allocator.h"
#include "parser_identifier.h"
#include "parser_type.h"
#include "parser_utils.h"
#include <stdio.h>

// `child1` points to the type and `child2` to the scope if there is an initialization
 
NodeSubtypeIdentificationBitCommand parser_identification_token_subtype_TO_node_subtype_identification_bit_command(TokenSubtype subtype_token) {
	switch(subtype_token) {
	case TokenSubtype_HASH: return NodeSubtypeIdentificationBitCommand_HASH;
	case TokenSubtype_AT: return NodeSubtypeIdentificationBitCommand_AT;
	case TokenSubtype_EXCLAMATION_MARK: return NodeSubtypeIdentificationBitCommand_EXCLAMATION_MARK;
	default: assert(false);
	}
}

static NodeSubtypeLiteral token_subtype_literal_to_subtype(TokenSubtype subtype_token) {
	return (NodeSubtypeLiteral) subtype_token;
}

static int if_declaration_create_nodes(
size_t* i,
MemoryArea* restrict memArea,
Node** node_identification,
Parser* parser) {
	assert(i != NULL);
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

	subtype |= parser_identification_token_subtype_TO_node_subtype_identification_bit_command(tokens[buffer_i].subtype);
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
		.type = NodeType_IDENTIFICATION,
		.subtype = subtype,
		.token = tokens + buffer_i,
		.child1 = NULL,
		.child2 = NULL};
	buffer_i += 1;

	if(node_identification != NULL)
		*node_identification = (Node*) parser->nodes.top;

	while(parser_is_qualifier(tokens + i_qualifier)) {
		if(!parser_allocator(parser))
			return -1;

		*((Node*) parser->nodes.top) = (Node) {
			.is_child = true,
			.type = NodeType_QUALIFIER,
			.subtype = tokens[i_qualifier].subtype,
			.token = tokens + i_qualifier,
			.child1 = NULL,
			.child2 = NULL};
		((Node*) parser->nodes.previous)->child1 = (Node*) parser->nodes.top;
		i_qualifier += 1;
	}
	// type deduction later
	NodeSubtypeIdentificationBitScoped bit_scoped;
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
		if(node_identification != NULL)
			(*node_identification)->subtype |= bit_scoped;
		break;
	}
	
	*i = buffer_i;
	return 1;
}

static int if_initialization_create_node(
bool nodes_initialization,
size_t* i,
Node* node_identification,
Parser* parser) {
	// just parse literals for the moment, expressions later
	const Token* tokens = (const Token*) parser->lexer->tokens.addr;
	size_t buffer_i = *i;

	if(parser_is_scope_L(tokens + buffer_i))
		return 1; // `.child2` determined in the loop of `create_parser`

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
			.token = tokens + buffer_i};
		buffer_i += 1;
	} else {
		memory_chain_state_restore(
			&parser->nodes,
			&memChain_state);
		return 0;
	}

	node_identification->child2 = (Node*) parser->nodes.top;
NODES_NO:
	*i = buffer_i;
	return 1;
}

int if_identification_create_nodes(
bool nodes_initialization,
size_t* i,
MemoryArea* restrict memArea,
Node** node_identification,
Parser* parser) {
	assert(i != NULL);
	assert(memArea != NULL);
	assert(node_identification != NULL);
	assert(parser != NULL);

	size_t buffer_i = *i;

	switch(if_declaration_create_nodes(
		&buffer_i,
		memArea,
		node_identification,
		parser)) {
	case -1: return -1;
	case 0: return 0;
	}
	// add the type as child nodes in `.child1`	
	switch(if_initialization_create_node(
		nodes_initialization,
		&buffer_i,
		*node_identification,
		parser)) {
	case -1: return -1;
	case 0: // declaration case
		(*node_identification)->subtype |= NodeSubtypeIdentificationBitType_DECLARATION;
		break;
	case 1: // initialization case
		(*node_identification)->subtype |= NodeSubtypeIdentificationBitType_INITIALIZATION;
		break;
	}

	*i = buffer_i;
	return 1;
}

bool parser_is_identification(const Node* node) {
	return node->type == NodeType_IDENTIFICATION;
}

bool parser_identification_is_declaration(const Node* node) {
	return (node->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_TYPE)
	    == NodeSubtypeIdentificationBitType_DECLARATION;
}

bool parser_identification_is_initialization(const Node* node) {
	return (node->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_TYPE)
	    == NodeSubtypeIdentificationBitType_INITIALIZATION;
}

bool parser_identification_is_label(const Node* node) {
	return (node->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_SCOPED)
	    == NodeSubtypeIdentificationBitScoped_LABEL;
}

bool parser_identification_is_label_parameterized(const Node* node) {
	return (node->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_SCOPED)
		 == NodeSubtypeIdentificationBitScoped_LABEL_PARAMETERIZED;
}
