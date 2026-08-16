#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "types.h"

#define MAX_STMTS 100
#define MAX_FUNCS 10
#define MAX_SYMS 10

struct sym {
	char name[32];
	enum data_type type;
};

struct symtable {
	struct sym table[MAX_SYMS];
	int pos;
};

struct parser {
	struct token *tokens;
	int pos;
	struct symtable symtable;
};

static struct token peek_multiple(struct parser *p, int i)
{
	return p->tokens[p->pos + i - 1];
}

static struct token peek(struct parser *p)
{
	return peek_multiple(p, 1);
}

static struct token advance(struct parser *p)
{
	return p->tokens[p->pos++];
}

static struct token expect(struct parser *p, enum token_type t)
{
	if (peek(p).type == t) {
		return advance(p);
	} else {
		fprintf(stderr, "parsing error\n");
		exit(1);
	}
}

static enum data_type symtable_load(struct symtable *s, char *name)
{
	for (int i = 0; i < s->pos; i++) {
		if (!strcmp(s->table[i].name, name)) {
			return s->table[i].type;
		}
	}
	return DTYPE_ERR;
}

static void symtable_store(struct symtable *s, char *name, enum data_type type)
{
	if (s->pos == MAX_SYMS) {
		fprintf(stderr, "too many symbols\n");
		exit(1);
	}
	if (symtable_load(s, name) != DTYPE_ERR) {
		fprintf(stderr, "duplicate symbol %s\n", name);
		exit(1);
	}
	strncpy(s->table[s->pos].name, name, MAX_ID_LEN - 1);
	s->table[s->pos].name[MAX_ID_LEN - 1] = '\0';
	s->table[s->pos++].type = type;
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

int get_precedence(enum token_type tt)
{
	switch (tt) {
	case TOKEN_PLUS:
		return 11;
	case TOKEN_STAR:
		return 12;
	default:
		return -1;
	}
}

struct node *parse_primary(struct parser *p)
{
	struct node *n;
	if (peek(p).type == TOKEN_INT_LIT) {
		n = node_create(0, NODE_INT_LIT);
		n->val = expect(p, TOKEN_INT_LIT).val;
		n->d_type = DTYPE_INT;
	} else if (peek(p).type == TOKEN_ID) {
		n = node_create(0, NODE_VAR_NAME);
		strncpy(n->name, expect(p, TOKEN_ID).name, MAX_ID_LEN - 1);
		n->name[MAX_ID_LEN - 1] = '\0';
		enum data_type d = symtable_load(&p->symtable, n->name);
		if (d == DTYPE_ERR) {
			fprintf(stderr, "unknown symbol %s\n", n->name);
			exit(1);
		}
		n->d_type = d;
	} else {
		fprintf(stderr, "parsing error\n");
		exit(1);
	}
	return n;
}

struct node *parse_expr(struct parser *p, int prec_min)
{
	struct node *left = parse_primary(p);
	for (;;) {
		struct token next = peek(p);
		int prec_next = get_precedence(next.type);
		if (prec_next < prec_min) {
			break;
		}
		advance(p);
		struct node *right = parse_expr(p, prec_next + 1);
		struct node *parent = node_create(2, NODE_BIN);
		parent->children_num = 2;
		parent->children[0] = left;
		parent->children[1] = right;
		left = parent;
		switch (next.type) {
		case TOKEN_PLUS:
			strcpy(left->op, "+");
			break;
		case TOKEN_STAR:
			strcpy(left->op, "*");
			break;
		default:
			strcpy(left->op, "?");
			break;
		}
	}
	return left;
}

static struct node *parse_stmt(struct parser *p)
{
	// Shared
	bool declare = false;
	if (peek(p).type == TOKEN_INT) {
		declare = true;
		advance(p);
	}
	char var_name[32];
	strcpy(var_name, expect(p, TOKEN_ID).name);
	if (declare) {
		symtable_store(&p->symtable, var_name, DTYPE_INT);
	} else {
		if (symtable_load(&p->symtable, var_name) == DTYPE_ERR) {
			fprintf(stderr, "unknown symbol %s\n", var_name);
			exit(1);
		}
	}
	struct token t = peek(p);
	if (t.type == TOKEN_SEMI) {
		advance(p);
		return NULL;
	}

	// If assignment
	struct node *parent = node_create(2, NODE_ASSIGN);
	parent->children_num = 2;
	expect(p, TOKEN_ASSIGN);
	struct node *left = node_create(0, NODE_VAR_NAME);
	parent->children[0] = left;
	strcpy(left->name, var_name);
	left->d_type = DTYPE_INT;

	parent->children[1] = parse_expr(p, 0);
	expect(p, TOKEN_SEMI);
	return parent;
}

static struct node *parse_func_decl(struct parser *p)
{
	expect(p, TOKEN_VOID);
	struct node *parent = node_create(MAX_STMTS, NODE_FUNC);
	parent->d_type = DTYPE_VOID;
	strcpy(parent->name, expect(p, TOKEN_ID).name);
	expect(p, TOKEN_LPAREN);
	expect(p, TOKEN_VOID);
	expect(p, TOKEN_RPAREN);
	expect(p, TOKEN_LBRACE);

	for (int i = 0; peek(p).type != TOKEN_RBRACE; i++) {
		struct node *n = parse_stmt(p);
		if (!n) {
			i--;
		} else {
			parent->children[i] = n;
			parent->children_num++;
		}
	}
	advance(p);
	return parent;
}

struct node *parse_program(struct parser *p)
{
	struct node *parent = node_create(MAX_FUNCS, NODE_PROG);
	parent->d_type = DTYPE_ROOT;
	for (int i = 0; peek(p).type != TOKEN_EOF; i++) {
		parent->children[i] = parse_func_decl(p);
		parent->children_num++;
	}
	advance(p);
	return parent;
}

struct parser *parser_create_and_load(struct token *tokens)
{
	struct parser *p = calloc(1, sizeof(*p));
	p->tokens = malloc(MAX_TOKENS * sizeof(*p->tokens));
	memcpy(p->tokens, tokens, MAX_TOKENS * sizeof(*p->tokens));
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
		printf("%s FUNC %s\n", (n->d_type == DTYPE_VOID) ? "VOID" : "INT", n->name);
		break;
	case NODE_ASSIGN:
		printf("ASSIGN\n");
		break;
	case NODE_VAR_NAME:
		printf("%s VAR %s\n", (n->d_type == DTYPE_VOID) ? "VOID" : "INT", n->name);
		break;
	case NODE_INT_LIT:
		printf("INT %d\n", n->val);
		break;
	case NODE_BIN:
		printf("BIN %s\n", n->op);
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

void ast_free(struct node *n)
{
	for (int i = 0; i < n->children_num; i++) {
		ast_free(n->children[i]);
	}
	free(n->children);
	free(n);
}
