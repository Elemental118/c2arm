#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

enum node_type {
	NODE_ERR,

	NODE_PROG,
	NODE_BIN,
	NODE_FUNC,
	NODE_INT_LIT,
	NODE_VAR_NAME,
};

struct node {
	int children_num;
	struct node **children;
	enum node_type type;
	union {
		char op;
		int  val;
		char name[32];
	};
};

struct parser *parser_create_and_load(struct token *tokens, int token_num);
void parser_free(struct parser *p);
struct node *parse_program(struct parser *p);
void ast_print(struct node *n);

#endif
