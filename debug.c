#ifndef NDEBUG
#include <inttypes.h>
#include <stdio.h>
#include "debug.h"

static void print_info_token(
const char* code,
const Token* restrict token) {
	const char* type;

	switch(token->type) {
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

	if(token->type == TokenType_COMMAND
	|| token->type == TokenType_SPECIAL
	|| token->type == TokenType_QL
	|| token->type == TokenType_L
	|| token->type == TokenType_PL
	|| token->type == TokenType_IDENTIFIER) {
		printf("%.*s\n",
			token->L_end - token->L_start,
			&code[token->L_start]);
	} else if(token->type == TokenType_QR
	       || token->type == TokenType_R) {
		printf("%.*s\n",
			token->R_end - token->R_start,
			&code[token->R_start]);
	} else if(token->type == TokenType_QLR
	       || token->type == TokenType_LR) {
		printf("%.*s, %.*s\n",
			token->L_end - token->L_start,
			&code[token->L_start],
			token->R_end - token->R_start,
			&code[token->L_start]);
	} else if(token->type == TokenType_LITERAL) {
		switch(token->subtype) {
		case TokenSubtype_LITERAL_NUMBER:
			printf("NUM %.*s\n",
				token->L_end - token->L_start,
				&code[token->L_start]); break;
		case TokenSubtype_LITERAL_STRING:
			printf("STR %.*s\n",
				token->L_end - token->L_start,
				&code[token->L_start]); break;
		case TokenSubtype_LITERAL_ASCII:
			printf("CHR %c\n",
				&code[token->L_start]);
		}
	}
}

static void print_info_node_identification(
const char* code,
const Node* node) {
	if((node->subtype & MASK_NODE_SUBTYPE_COMMAND) == NodeSubtypeKeyQualification_HASH)
		printf("# ");
	else
		printf("@ ");

	printf("DECLARATION <%.*s>",
		node->token->L_end - node->token->L_start,
		&code[node->token->L_start]);

	if((node->subtype & NodeSubtypeKeyQualification_ENTRY) != 0)
		printf(" ENTRY");

	if((node->subtype & NodeSubtypeKeyQualification_INC) != 0)
		printf(" INC");

	if((node->subtype & NodeSubtypeKeyQualification_MUT) != 0)
		printf(" MUT");

	printf("\n");
}

void debug_print_tokens(const Lexer* lexer) {
	printf("TOKENS:\n");

	for(long int i = 0;
	i < lexer->count;
	i += 1) {
		printf("\t");
		print_info_token(
			lexer->source->content,
			&lexer->tokens[i]);
	}

	printf(
		"\nNumber of tokens: %d.\n",
		lexer->count);
}

void debug_print_nodes(const Parser* parser) {
	const char* code = parser->lexer->source->content;
	printf("NODES:\n");

	for(long int j = 0;
	j < parser->count;
	j += 1) {
		const Node* node = &parser->nodes[j];

		if(node->type == NodeType_SCOPE_START) {
			printf("\tSCOPE START (%td NODES)\n",
				node->child - node);
		} else if(node->type == NodeType_IDENTIFICATION) {
			const Node* child = node->child;
			printf("\t");
			print_info_node_identification(
				code,
				node);

			do {
				printf("\t\t");
				print_info_token(
					code,
					child->token);
				child = child->child1;
				j += 1;
			} while(child != NULL);
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
		"\nNumber of nodes: %d.\n",
		parser->count);
}

#endif
