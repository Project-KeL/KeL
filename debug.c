#ifndef NDEBUG
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include "debug.h"

static void print_info_token(
const char* code,
const Token* token) {
	const char* type;

	switch(token->type) {
	case TokenType_COLON_LONELY: type = "COL"; break;
	case TokenType_COMMAND: type = "COM"; break;
	case TokenType_SPECIAL: type = "SPE"; break;
	case TokenType_QL: type = "QL "; break;
	case TokenType_QR: type = "QR "; break;
	case TokenType_QLR: type = "QLR"; break;
	case TokenType_L: type = "L  "; break;
	case TokenType_R: type = "R  "; break;
	case TokenType_LR: type = "LR "; break;
	case TokenType_PL: type = "PL "; break;
	case TokenType_IDENTIFIER: type = "ID "; break;
	case TokenType_LITERAL: type = "LIT"; break;
	}

	if(token->subtype != TokenSubtype_SCOPE) {
		printf("%s \t ",
			type);
	}

	if(token->type == TokenType_COLON_LONELY
	|| token->type == TokenType_COMMAND
	|| token->type == TokenType_SPECIAL
	|| token->type == TokenType_QL
	|| token->type == TokenType_L
	|| token->type == TokenType_PL
	|| token->type == TokenType_IDENTIFIER) {
		if(token->subtype == TokenSubtype_MODULE_INPUT) {
			printf("IMOD\n");
		} else if(token->subtype == TokenSubtype_SCOPE) {
			printf("L SCOPE\n");
		} else {
			printf("%.*s\n",
				(int) (token->L_end - token->L_start),
				code + token->L_start);
		}
	} else if(token->type == TokenType_QR
	       || token->type == TokenType_R) {
		if(token->subtype == TokenSubtype_SCOPE) {
			printf("R SCOPE\n");
		} else {
			printf("%.*s\n",
				(int) (token->R_end - token->R_start),
				code + token->R_start);
		}
	} else if(token->type == TokenType_QLR) {
		if(token->subtype == TokenType_QL) {
			printf("%.*s\n",
				(int) (token->L_end - token->L_start),
				code + token->L_start);
		} else {
			printf("%.*s\n",
				(int) (token->R_end - token->R_start),
				&code[token->R_start]);
		}
	} else if(token->type == TokenType_LR) {
		printf("%.*s, %.*s\n",
			(int) (token->L_end - token->L_start),
			code + token->L_start,
			(int) (token->R_end - token->R_start),
			code + token->L_start);
	} else if(token->type == TokenType_LITERAL) {
		switch(token->subtype) {
		case TokenSubtype_LITERAL_NUMBER:
			printf("NUM"); break;
		case TokenSubtype_LITERAL_STRING:
			printf("STR"); break;
		case TokenSubtype_LITERAL_CHARACTER:
			printf("CHR"); break;
		}

		printf("\t<%.*s>\n",
			(int) (token->end - token->start),
			code + token->start);
	}
}

static void print_info_node_key_identification(
const char* code,
const Node* node) {
	bool is_initialization = false;

	if((node->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_COMMAND)
	== NodeSubtypeIdentificationBitCommand_HASH)
		printf("# ");
	else if((node->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_COMMAND)
	== NodeSubtypeIdentificationBitCommand_AT)
		printf("@ ");
	else if((node->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_COMMAND)
	== NodeSubtypeIdentificationBitCommand_EXCLAMATION_MARK)
		printf("! ");

	if((node->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_TYPE)
	== NodeSubtypeIdentificationBitType_DECLARATION) {
		printf("DECLARATION");
	} else {
		is_initialization = true;
		printf("INITIALIZATION:");
	}

	printf(" <%.*s>",
		(int) (node->token->L_end - node->token->L_start),
		code + node->token->L_start);

	if((node->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_SCOPED)
	== NodeSubtypeIdentificationBitScoped_LABEL) {
		printf(" LABEL");
	} else if((node->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_SCOPED)
	== NodeSubtypeIdentificationBitScoped_LABEL_PARAMETERIZED) {
		printf(" PARAMETERIZED LABEL");
	} else if(is_initialization) {
		printf(" <%.*s>",
			(int) (node->child2->token->L_end - node->child2->token->L_start),
			code + node->child2->token->L_start);
	}

	printf("\n");
}

static void print_info_node_type(
const char* code,
const Node* node) {
	const Token* token = node->token;

	switch(node->type) {
	case NodeType_QUALIFIER:
		if(node->subtype == TokenType_QL) {
			printf("QL <%.*s>\n",
				(int) (token->L_end - token->L_start),
				code + token->L_start);
		} else {
			printf("QR <%.*s>\n",
				(int) (token->R_end - token->R_start),
				code + token->R_start);
		} break;
	case NodeTypeChildType_LOCK:
		switch(node->subtype) {
			case NodeSubtypeChild_NO:
				printf("LOCK <%.*s>\n",
					(int) (token->R_end - token->R_start),
					code + token->R_start); break;
			case NodeSubtypeChildTypeScoped_RETURN_NONE:
				printf("RETURN NONE\n"); break;
			case NodeSubtypeChildTypeScoped_RETURN_TYPE:
				printf("RETURN TYPE <%.*s>\n",
					(int) (token->R_end - token->R_start),
					code + token->R_start); break;
			case NodeSubtypeChildTypeScoped_PARAMETER_NONE:
				printf("PARAMETER NONE\n"); break;
			case NodeSubtypeChildTypeScoped_PARAMETER:
				printf("PARAMETER <%.*s>\n",
					(int) (token->L_end - token->L_start),
					code + token->L_start); break;
		} break;
	}
}

void debug_print_tokens(const Lexer* lexer) {
	printf("TOKENS:\n");

	for(size_t i = 1;
	i < lexer->tokens.count - 1;
	i += 1) {
		printf("\t");
		print_info_token(
			lexer->source->content,
			(Token*) lexer->tokens.addr + i);
	}

	printf(
		"\nNumber of tokens: %zu.\n",
		lexer->tokens.count);
}

void debug_print_declarations(const Parser* parser) {
	const char* code = parser->lexer->source->content;
	MemoryChainLink* link;
	const Node* node;

	if(parser->declarations.first->memArea.count > 1) {
		link = parser->declarations.first;
		node = (Node*) link->memArea.addr + 1;
	} else {
		link = parser->declarations.first->next;
		node = (Node*) link->memArea.addr;
	}
	
	size_t count = 0;
	printf("DECLARATIONS:\n");

	while(node != (Node*) parser->declarations.last->memArea.addr + parser->declarations.last->memArea.count) {
		if(node->type == NodeType_NO)
			break;

		printf("\t");

		print_info_node_key_identification(
			parser->lexer->source->content,
			node);
		const Node* child1 = node->child1;

		do {
			if(node == (Node*) link->memArea.addr + link->memArea.count - 1)
				link = link->next;

			printf("\t\t");
			print_info_node_type(
				code,
				child1);
			node = child1;
			child1 = node->child1;
		} while(child1 != NULL);

		count += 1;

		if(node == (Node*) link->memArea.addr + link->memArea.count - 1) {
			if(link->next == NULL)
				break;

			link = link->next;
			node = (Node*) link->memArea.addr;
		} else
			node += 1;
	}

	printf("\nNumber of declarations at file scope: %zu\n", count);
}

void debug_print_nodes(const Parser* parser) {
	const char* code = parser->lexer->source->content;
	MemoryChainLink* link;
	const Node* node;

	if(parser->nodes.first->memArea.count > 1) {
		link = parser->nodes.first;
		node = (Node*) link->memArea.addr + 1;
	} else {
		link = parser->nodes.first->next;
		node = (Node*) link->memArea.addr;
	}

	size_t count = 0;
	printf("NODES:\n");

	while(node != (Node*) parser->nodes.last->memArea.addr + parser->nodes.last->memArea.count) {
		printf("\t");

		if(node->type == NodeType_MODULE) {
			if(node->subtype == NodeSubtypeModule_INPUT)
				printf("IMOD\t");
			else if(node->subtype == NodeSubtypeModule_OUTPUT)
				printf("OMOD\t");

			printf("<%.*s>\n",
				(int) (node->token->L_end - node->token->L_start),
				code + node->token->L_start);
			const Node* child = node->child;

			while(child != NULL) {
				if(node == (Node*) link->memArea.addr + link->memArea.count - 1)
					link = link->next;

				printf("\t\tSUBMOD <%.*s>\n",
					(int) (child->token->L_end - child->token->L_start),
					code + child->token->L_start);
				node = child;
				child = child->child;
				count += 1;
			}

			count += 1;
		} else if(node->type == NodeType_SCOPE_START) {
			printf("SCOPE START (%td NODES)\n",
				node->value - 1);
			count += 1;
		} else if(node->type == NodeType_IDENTIFICATION) {
			print_info_node_key_identification(
				code,
				node);
			const Node* child1 = node->child1;

			do {
				if(node == (Node*) link->memArea.addr + link->memArea.count - 1)
					link = link->next;

				printf("\t\t");
				print_info_node_type(
					code,
					child1);
				node = child1;
				child1 = node->child1;
				count += 1;
			} while(child1 != NULL);

			count += 1;
		} else if(node->type == NodeType_SCOPE_END) {
			printf("SCOPE END\n");
			count += 1;
		} else if(node->type == NodeType_LITERAL) {
			print_info_token(
				code,
				node->token);
			count += 1;
		} else {
			printf(
				"%" PRIu64 ", %" PRIu64 "\n",
				node->type,
				node->subtype);
			count += 1;
		}

		if(node == (Node*) link->memArea.addr + link->memArea.count - 1) {
			if(link->next == NULL)
				break;

			link = link->next;
			node = (Node*) link->memArea.addr;
		} else
			node += 1;
	}

	printf(
		"\nNumber of nodes: %zu.\n",
		count);
}

#endif
