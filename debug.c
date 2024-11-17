#ifndef NDEBUG
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include "debug.h"
#include "parser_allocator.h"
#include "parser_call.h"
#include "parser_introduction.h"
#include "parser_module.h"
#include "parser_type.h"

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
	|| token->type == TokenType_PL) {
		if(token->subtype == TokenSubtype_IDENTIFIER)
			printf("ID ");

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

static void print_info_node_key_introduction(
const char* code,
const Node* node) {
	bool is_initialization = false;

	if((node->subtype & MASK_BIT_NODE_SUBTYPE_INTRODUCTION_COMMAND)
	== NodeSubtypeIntroductionBitCommand_HASH)
		printf("# ");
	else if((node->subtype & MASK_BIT_NODE_SUBTYPE_INTRODUCTION_COMMAND)
	== NodeSubtypeIntroductionBitCommand_AT)
		printf("@ ");
	else if((node->subtype & MASK_BIT_NODE_SUBTYPE_INTRODUCTION_COMMAND)
	== NodeSubtypeIntroductionBitCommand_EXCLAMATION_MARK)
		printf("! ");

	if((node->subtype & MASK_BIT_NODE_SUBTYPE_INTRODUCTION_TYPE)
	== NodeSubtypeIntroductionBitType_DECLARATION) {
		printf("DEC");
	} else {
		is_initialization = true;
		printf("INI");
	}

	printf(" <%.*s>",
		(int) (node->token->L_end - node->token->L_start),
		code + node->token->L_start);

	if((node->subtype & MASK_BIT_NODE_SUBTYPE_INTRODUCTION_SCOPED)
	== NodeSubtypeIntroductionBitScoped_LABEL) {
		printf(" LAB");
	} else if((node->subtype & MASK_BIT_NODE_SUBTYPE_INTRODUCTION_SCOPED)
	== NodeSubtypeIntroductionBitScoped_PAL) {
		printf(" PLAB");
	}
	
	Node* initialization = node->nodes[NODE_INDEX_INTRODUCTION_INITIALIZATION];

	if(is_initialization) {
		if(node->subtype & MASK_BIT_NODE_SUBTYPE_INTRODUCTION_SCOPED) {
			printf(
				" (SCOPE ID: %p)",
				initialization);
		} else {
			printf(
				" <%.*s>",
				(int) (initialization->token->L_end - initialization->token->L_start),
				code + initialization->token->L_start);
		}
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
			printf("QL <%.*s>",
				(int) (token->L_end - token->L_start),
				code + token->L_start);
		} else {
			printf("QR <%.*s>",
				(int) (token->R_end - token->R_start),
				code + token->R_start);
		} break;
	case NodeTypeChildType_LOCK:
		switch(node->subtype) {
			case NodeSubtypeChild_NO:
				printf("TYPE <%.*s>",
					(int) (token->R_end - token->R_start),
					code + token->R_start); break;
			case NodeSubtypeChildTypeScoped_RETURN_NONE:
				printf("RETURN NONE"); break;
			case NodeSubtypeChildTypeScoped_RETURN_TYPE:
				printf("RETURN TYPE <%.*s>",
					(int) (token->R_end - token->R_start),
					code + token->R_start); break;
			case NodeSubtypeChildTypeScoped_PARAMETER_NONE:
				printf("PARAMETER NONE"); break;
			case NodeSubtypeChildTypeScoped_PARAMETER:
				printf("PARAMETER <%.*s>",
					(int) (token->L_end - token->L_start),
					code + token->L_start); break;
		} break;
	}

	if(node->type == NodeTypeChild_NO)
		printf("NULL");

	printf("\n");
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

void debug_print_file_nodes(const Parser* parser) {
	const char* code = parser->lexer->source->content;
	const MemoryChainLink* link;
	const Node* node = parser_allocator_start_file_node(
		parser,
		&link);
	printf("DECLARATIONS:\n");

	if(node == NULL)
		return;

	do {
		printf("\t");

		if(node->type == NodeType_NO)
			printf("NULL\n");

		print_info_node_key_introduction(
			parser->lexer->source->content,
			node);
		parser_allocator_next(
			parser,
			&link,
			&node);

		do {
			parser_allocator_next(
				parser,
				&link,
				&node);
			printf("\t\t");
			print_info_node_type(
				code,
				node);
		} while(node->type != NodeType_NO);
	} while(parser_allocator_next(
		parser,
		&link,
		&node)
	== true);

	printf("\nNumber of introductions at file scope: %zu\n", parser->count_file_nodes);
}

void print_info_node_key_call(
const char* code,
const Node* node) {
	printf("CALL <%.*s>\n",
		(int) (node->token->L_end - node->token->L_start),
		code + node->token->L_start);
}

void print_info_node_key_call_return_type(
const char* code,
const Node* node) {
	if(node->type == NodeTypeChildCall_RETURN_UNKNOWN) {
		printf("RETURN UNKNOWN");
	} else if(node->type == NodeTypeChildCall_RETURN_TYPE) {
		printf("RETURN TYPE <%.*s>",
			(int) (node->token->L_end - node->token->L_start),
			code + node->token->L_start);
	}

	printf("\n");
}

void print_info_node_argument(
const char* code,
const Node* node) {
	switch(node->type) {
	case NodeTypeChildCall_ARGUMENT_NONE:
		printf("ARGUMENT NONE"); break;
	case NodeTypeChildCall_ARGUMENT:
		printf("<%.*s>",
			(int) (node->token->L_end - node->token->L_start),
			code + node->token->L_start); break;
	}

	if(node->type == NodeTypeChild_NO)
		printf("NULL");

	printf("\n");
}

void debug_print_nodes(const Parser* parser) {
	const char* code = parser->lexer->source->content;
	const MemoryChainLink* link;
	const Node* node = parser_allocator_start_node(
		parser,
		&link);
	printf("NODES:\n");

	if(node == NULL)
		return;

	do {
		printf("\t");

		if(node->type == NodeType_MODULE) {
			parser_allocator_next(
				parser,
				&link,
				&node);

			if(node->subtype == NodeSubtypeModule_INPUT)
				printf("IMOD ");
			else if(node->subtype == NodeSubtypeModule_OUTPUT)
				printf("OMOD ");

			printf("<%.*s>\n",
				(int) (node->token->L_end - node->token->L_start),
				code + node->token->L_start);
			const Node* tail = parser_module_get_tail(node);

			while(tail != NULL) {
				parser_allocator_next(
					parser,
					&link,
					&node);
				printf("\t\tSUBMOD <%.*s>\n",
					(int) (tail->token->L_end - tail->token->L_start),
					code + tail->token->L_start);
				node = tail;
				tail = parser_module_get_tail(tail);
			}
		} else if(node->type == NodeType_SCOPE_START) {
			printf(
				"SCOPE START (%td NODES) ID: %p\n",
				node->value - 1,
				node);
		} else if(node->type == NodeType_INTRODUCTION) {
			print_info_node_key_introduction(
				code,
				node);
			parser_allocator_next(
				parser,
				&link,
				&node);

			do {
				parser_allocator_next(
					parser,
					&link,
					&node);
				printf("\t\t");
				print_info_node_type(
					code,
					node);
			} while(parser_introduction_get_type(node) != NULL);
		} else if(node->type == NodeType_CALL) {
			print_info_node_key_call(
				code,
				node);
			parser_allocator_next(
				parser,
				&link,
				&node);
			printf("\t\t");
			print_info_node_key_call_return_type(
				code,
				node);

			do {
				parser_allocator_next(
					parser,
					&link,
					&node);	
				printf("\t\t");
				print_info_node_argument(
					code,
					node);
			} while(parser_call_get_tail(node) != NULL);
			// null node
		} else if(node->type == NodeType_SCOPE_END) {
			printf("SCOPE END\n");
		} else if(node->type == NodeType_LITERAL) {
			print_info_token(
				code,
				node->token);
		} else {
			printf(
				"%" PRIu64 ", %" PRIu64 "\n",
				node->type,
				node->subtype);
		}
	} while(parser_allocator_next(
		parser,
		&link,
		&node)
	== true);

	printf(
		"\nNumber of nodes: %zu.\n",
		parser->count_nodes);
}

#endif
