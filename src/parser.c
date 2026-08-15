#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "types.h"

#define MAX_STMTS 100

struct parser {
	struct token *tokens;
	int pos;
};

static struct token peek(struct parser *p)
{
	return p->tokens[p->pos];
}

static struct token advance(struct parser *p)
{
	return p->tokens[p->pos++];
}

static void expect(struct parser *p, enum token_type t)
{
	if (peek(p).type == t) {
		advance(p);
	} else {
		fprintf(stderr, "parsing error\n");
		exit(1);
	}
}

static struct node *node_create(int children_num, enum node_type nt)
{
	struct node *n = calloc(1, sizeof(*n));
	n->n_type = nt;
	if (children_num) {
		n->children = malloc(children_num * sizeof(*n->children));
	}
	return n;
}

static struct node *parse_var_decl(struct parser *p)
{
	struct node *parent = node_create(2, NODE_BIN);
	parent->op = '=';
	parent->d_type = DTYPE_INT;
	expect(p, TOKEN_INT);
	struct node *left = node_create(0, NODE_VAR_NAME);

	parent->children[0] = left;
	strcpy(left->name, peek(p).name);
	left->d_type = DTYPE_INT;
	expect(p, TOKEN_ID);
	expect(p, TOKEN_ASSIGN);

	struct node *right = node_create(0, NODE_INT_LIT);
	parent->children[1] = right;
	right->d_type = DTYPE_INT;
	right->val = peek(p).val;
	expect(p, TOKEN_INT_LIT);
	expect(p, TOKEN_SEMI);
	parent->children_num = 2;
	return parent;
}

static struct node *parse_func_decl(struct parser *p)
{
	expect(p, TOKEN_VOID);
	struct node *parent = node_create(MAX_STMTS, NODE_FUNC);
	parent->d_type = DTYPE_VOID;
	strcpy(parent->name, peek(p).name);
	expect(p, TOKEN_ID);
	expect(p, TOKEN_LPAREN);
	expect(p, TOKEN_VOID);
	expect(p, TOKEN_RPAREN);
	expect(p, TOKEN_LBRACE);

	for (int i = 0; peek(p).type != TOKEN_RBRACE; i++) {
		parent->children[i] = parse_var_decl(p);
		parent->children_num++;
	}
	advance(p);
	return parent;
}

struct node *parse_program(struct parser *p)
{
	struct node *parent = node_create(MAX_STMTS, NODE_PROG);
	parent->d_type = DTYPE_ROOT;
	for (int i = 0; peek(p).type != TOKEN_EOF; i++) {
		parent->children[i] = parse_func_decl(p);
		parent->children_num++;
	}
	advance(p);
	return parent;
}

struct parser *parser_create_and_load(struct token *tokens, int token_num)
{
	struct parser *p = calloc(1, sizeof(*p));
	p->tokens = malloc(token_num * sizeof(*p->tokens));
	memcpy(p->tokens, tokens, token_num * sizeof(*p->tokens));
	return p;
}

void parser_free(struct parser *p)
{
	free(p->tokens);
	free(p);
}

static void ast_print_helper(struct node *n, int depth)
{
	for (int i = 0; i < depth; i++) {
		printf(" ");
	}

	switch (n->n_type) {
	case NODE_PROG:
		printf("PROG\n");
		break;
	case NODE_FUNC:
		printf("FUNC %s\n", n->name);
		break;
	case NODE_BIN:
		printf("BIN %c\n", n->op);
		break;
	case NODE_VAR_NAME:
		printf("VAR %s\n", n->name);
		break;
	case NODE_INT_LIT:
		printf("INT %d\n", n->val);
		break;
	case NODE_ERR:
	default:
		printf("UNKNOWN\n");
		return;	
	}

	depth += 4;
	for (int i = 0; i < n->children_num; i++) {
		ast_print_helper(n->children[i], depth);
	}
}

void ast_print(struct node *n)
{
	ast_print_helper(n, 0);
}
