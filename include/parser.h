#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

enum node_type {
	NODE_ERR,

	NODE_PROG,
	NODE_UN,
	NODE_BIN,
	NODE_FUNC,
	NODE_BLOCK,
	NODE_INT_LIT,
	NODE_IF,
	NODE_VAR_NAME
};

enum data_type {
	DTYPE_ERR,

	DTYPE_ROOT,
	DTYPE_INT,
	DTYPE_VOID,
	DTYPE_BOOL
};

struct node {
	int children_num;
	struct node **children;
	enum node_type n_type;
	enum data_type d_type;
	int id;
	union {
		char op[3];
		int  val;
		char name[32];
	};
};

struct symtable;

struct parser *parser_create_and_load(struct token *tokens);
void parser_free(struct parser *p);
struct node *parse_program(struct parser *p);

void ast_print(struct node *n);
void ast_free(struct node *n);

#endif
