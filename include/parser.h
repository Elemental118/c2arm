#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

enum node_type {
	NODE_ERR,

	NODE_PROG,
	NODE_BIN,
	NODE_ASSIGN,
	NODE_FUNC,
	NODE_INT_LIT,
	NODE_VAR_NAME
};

enum data_type {
	DTYPE_ERR,

	DTYPE_ROOT,
	DTYPE_INT,
	DTYPE_VOID
};

struct node {
	int children_num;
	struct node **children;
	enum node_type n_type;
	enum data_type d_type;
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
