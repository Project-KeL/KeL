#ifndef NDEBUG
#include <inttypes.h>
#include <stdio.h>
#include "debug.h"

static void print_info_token(
const char* code,
const Token* restrict token) {
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

	printf("%s \t ",
		type);

	if(token->type == TokenType_COLON_LONELY
	|| token->type == TokenType_COMMAND
	|| token->type == TokenType_SPECIAL
	|| token->type == TokenType_QL
	|| token->type == TokenType_L
	|| token->type == TokenType_PL
	|| token->type == TokenType_IDENTIFIER) {
		printf("%.*s\n",
			(int) (token->L_end - token->L_start),
			&code[token->L_start]);
	} else if(token->type == TokenType_QR
	       || token->type == TokenType_R) {
		printf("%.*s\n",
			(int) (token->R_end - token->R_start),
			&code[token->R_start]);
	} else if(token->type == TokenType_QLR) {
		if(token->subtype == TokenType_QL) {
			printf("%.*s\n",
				(int) (token->L_end - token->L_start),
				&code[token->L_start]);
		} else {
			printf("%.*s\n",
				(int) (token->R_end - token->R_start),
				&code[token->R_start]);
		}
	} else if(token->type == TokenType_LR) {
		printf("%.*s, %.*s\n",
			(int) (token->L_end - token->L_start),
			&code[token->L_start],
			(int) (token->R_end - token->R_start),
			&code[token->L_start]);
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
			&code[token->start]);
	}
}

static void print_info_node_key_identification(
const char* code,
const Node* node) {
	if((node->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_COMMAND)
	== NodeSubtypeIdentificationBitCommand_HASH)
		printf("# ");
	else
		printf("@ ");

	if((node->subtype & MASK_BIT_NODE_SUBTYPE_IDENTIFICATION_TYPE)
	== NodeSubtypeIdentificationBitType_DECLARATION)
		printf("DECLARATION");
	else
		printf("INITIALIZATION:");

	printf(" <%.*s>\n",
		(int) (node->token->L_end - node->token->L_start),
		&code[node->token->L_start]);
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
				&code[token->L_start]);
		} else {
			printf("QR <%.*s>\n",
				(int) (token->R_end - token->R_start),
				&code[token->R_start]);
		} break;
	case NodeTypeChildType_LOCK:
		switch(node->subtype) {
			case NodeSubtypeChild_NO:
				printf("LOCK <%.*s>\n",
					(int) (token->R_end - token->R_start),
					&code[token->R_start]); break;
			case NodeSubtypeChildTypeScoped_RETURN_NONE:
				printf("RETURN NONE\n"); break;
			case NodeSubtypeChildTypeScoped_RETURN_LOCK:
				printf("RETURN LOCK <%.*s>\n",
					(int) (token->R_end - token->R_start),
					&code[token->R_start]); break;
			case NodeSubtypeChildTypeScoped_PARAMETER_NONE:
				printf("PARAMETER NONE\n"); break;
			case NodeSubtypeChildTypeScoped_PARAMETER:
				printf("PARAMETER <%.*s>\n",
					(int) (token->L_end - token->L_start),
					&code[token->L_start]); break;
			case NodeSubtypeChildTypeScoped_PARAMETER_LOCK:
				printf("PARAMETER LOCK <%.*s>\n",
					(int) (token->R_end - token->R_start),
					&code[token->R_start]); break;
		} break;
	}
}

void debug_print_tokens(const Lexer* lexer) {
	printf("TOKENS:\n");

	for(long int i = 1;
	i < lexer->count;
	i += 1) {
		printf("\t");
		print_info_token(
			lexer->source->content,
			&lexer->tokens[i]);
	}

	printf(
		"\nNumber of tokens: %ld.\n",
		lexer->count - 1);
}

void debug_print_nodes(const Parser* parser) {
	const char* code = parser->lexer->source->content;
	printf("NODES:\n");

	for(long int j = 1;
	j < parser->count;
	j += 1) {
		const Node* node = &parser->nodes[j];

		if(node->type == NodeType_SCOPE_START) {
			printf("\tSCOPE START (%td NODES)\n",
				node->child - node);
		} else if(node->type == NodeType_IDENTIFICATION) {
			printf("\t");
			print_info_node_key_identification(
				code,
				node);
			const Node* child1 = node->child1;

			do {
				printf("\t\t");
				print_info_node_type(
					code,
					child1);
				child1 = child1->child1;
				j += 1;
			} while(child1 != NULL);
		} else if(node->type == NodeType_SCOPE_END) {
			printf("\tSCOPE END\n");
		} else {
			printf(
				"\t%" PRIu64 ", %" PRIu64 "\n",
				node->type,
				node->subtype);
		}
	}

	printf(
		"\nNumber of nodes: %ld.\n",
		parser->count - 1);
}

#endif
