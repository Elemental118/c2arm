#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "types.h"

#define MAX_STMTS 100
#define MAX_FUNCS 10
#define MAX_SYMS 10
#define MAX_SYM_STACK 10

struct sym {
	char name[32];
	int id;
	enum data_type type;
};

struct symtable {
	struct sym table[MAX_SYMS];
	int pos;
	int stack[MAX_SYM_STACK];
	int sp;
	int id_counter;
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
		fprintf(stderr, "parsing error: expected %s, got %s\n", token_names[t], token_names[peek(p).type]);
		exit(1);
	}
}

static void sym_stack_push(struct symtable *s)
{
	s->stack[s->sp++] = s->pos;
}

static void sym_stack_pop(struct symtable *s)
{
	s->pos = s->stack[--s->sp];
}

static int sym_stack_peek(struct symtable *s)
{
	return s->stack[s->sp - 1];
}

static struct sym symtable_load_helper(struct symtable *s, char *name, bool within_scope)
{
	for (int i = s->pos - 1; i >= (within_scope ? sym_stack_peek(s) : 0); i--) {
		if (!strcmp(s->table[i].name, name)) {
			return s->table[i];
		}
	}
	struct sym err;
	err.type = DTYPE_ERR;
	return err;
}

static struct sym symtable_load(struct symtable *s, char *name)
{
	return symtable_load_helper(s, name, false);
}

static void symtable_store(struct symtable *s, char *name, enum data_type type)
{
	if (s->pos == MAX_SYMS) {
		fprintf(stderr, "too many symbols\n");
		exit(1);
	}
	if (symtable_load_helper(s, name, true).type != DTYPE_ERR) {
		fprintf(stderr, "duplicate symbol %s\n", name);
		exit(1);
	}
	strncpy(s->table[s->pos].name, name, MAX_ID_LEN - 1);
	s->table[s->pos].name[MAX_ID_LEN - 1] = '\0';
	s->table[s->pos].type = type;
	s->table[s->pos++].id = s->id_counter++;
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
	case TOKEN_ASSIGN:
		return 1;
	case TOKEN_LOR:
		return 4;
	case TOKEN_LAND:
		return 4;
	case TOKEN_PIPE:
		return 5;
	case TOKEN_XOR:
		return 6;
	case TOKEN_AMP:
		return 7;
	case TOKEN_LT:
	case TOKEN_GT:
	case TOKEN_LE:
	case TOKEN_GE:
		return 9;
	case TOKEN_EQ:
	case TOKEN_NE:
		return 10;
	case TOKEN_PLUS:
	case TOKEN_MINUS:
		return 11;
	case TOKEN_STAR:
	case TOKEN_DIV:
	case TOKEN_MOD:
		return 12;
	default:
		return -1;
	}
}

struct node *parse_fundamental(struct parser *p)
{
	struct node *n;
	if (peek(p).type == TOKEN_INT_LIT) {
		n = node_create(0, NODE_INT_LIT);
		n->val = expect(p, TOKEN_INT_LIT).val;
		n->d_type = DTYPE_INT;
	} else if (peek(p).type == TOKEN_ID) {
		n = node_create(0, NODE_VAR_NAME);
		struct token var = expect(p, TOKEN_ID);
		strncpy(n->name, var.name, MAX_ID_LEN - 1);
		n->name[MAX_ID_LEN - 1] = '\0';
		struct sym var_sym = symtable_load(&p->symtable, n->name);
		if (var_sym.type == DTYPE_ERR) {
			fprintf(stderr, "unknown symbol %s\n", n->name);
			exit(1);
		}
		n->d_type = var_sym.type;
		n->id = var_sym.id;
	} else {
		fprintf(stderr, "parsing error: unrecognized fundamental construct\n");
		exit(1);
	}
	return n;
}

struct node *parse_unary(struct parser *p)
{
	struct token next = peek(p);
	if (next.type == TOKEN_MINUS || next.type == TOKEN_BNOT || next.type == TOKEN_LNOT) {
		advance(p);
		struct node *n;
		n = node_create(1, NODE_UN);
		n->children_num = 1;
		n->children[0] = parse_unary(p);
		n->d_type = next.type == TOKEN_LNOT ? DTYPE_BOOL : DTYPE_INT;
		switch (next.type) {
		case TOKEN_MINUS:
			strcpy(n->op, "-");
			break;
		
		case TOKEN_BNOT:
			strcpy(n->op, "~");
			break;
		
		case TOKEN_LNOT:
			strcpy(n->op, "!");
			break;
		
		default:
			strcpy(n->op, "?");
			break;
		}
		return n;
	} else {
		return parse_fundamental(p);
	}
}

struct node *parse_expr(struct parser *p, int prec_min)
{
	struct node *left = parse_unary(p);
	for (;;) {
		struct token next = peek(p);
		int prec_next = get_precedence(next.type);
		if (prec_next < prec_min) {
			break;
		}
		advance(p);
		struct node *right = parse_expr(p, prec_next + (next.type != TOKEN_ASSIGN));
		struct node *parent = node_create(2, NODE_BIN);
		parent->children_num = 2;
		parent->children[0] = left;
		parent->children[1] = right;
		left = parent;
		switch (next.type) {
		case TOKEN_PLUS:
			strcpy(left->op, "+");
			left->d_type = DTYPE_INT;
			break;
		
		case TOKEN_MINUS:
			strcpy(left->op, "-");
			left->d_type = DTYPE_INT;
			break;
		
		case TOKEN_STAR:
			strcpy(left->op, "*");
			left->d_type = DTYPE_INT;
			break;
		
		case TOKEN_DIV:
			strcpy(left->op, "/");
			left->d_type = DTYPE_INT;
			break;
		
		case TOKEN_MOD:
			strcpy(left->op, "%");
			left->d_type = DTYPE_INT;
			break;
		
		case TOKEN_AMP:
			strcpy(left->op, "&");
			left->d_type = DTYPE_INT;
			break;
		
		case TOKEN_PIPE:
			strcpy(left->op, "|");
			left->d_type = DTYPE_INT;
			break;
		
		case TOKEN_XOR:
			strcpy(left->op, "^");
			left->d_type = DTYPE_INT;
			break;
		
		case TOKEN_LAND:
			strcpy(left->op, "&&");
			left->d_type = DTYPE_INT;
			break;
		
		case TOKEN_LOR:
			strcpy(left->op, "||");
			left->d_type = DTYPE_INT;
			break;
		
		case TOKEN_LT:
			strcpy(left->op, "<");
			left->d_type = DTYPE_BOOL;
			break;
		
		case TOKEN_GT:
			strcpy(left->op, ">");
			left->d_type = DTYPE_BOOL;
			break;
		
		case TOKEN_LE:
			strcpy(left->op, "<=");
			left->d_type = DTYPE_BOOL;
			break;
		
		case TOKEN_GE:
			strcpy(left->op, ">=");
			left->d_type = DTYPE_BOOL;
			break;
		
		case TOKEN_EQ:
			strcpy(left->op, "==");
			left->d_type = DTYPE_BOOL;
			break;
		
		case TOKEN_NE:
			strcpy(left->op, "!=");
			left->d_type = DTYPE_BOOL;
			break;
		
		case TOKEN_ASSIGN:
			strcpy(left->op, "=");
			left->d_type = right->d_type;
			break;
		
		default:
			strcpy(left->op, "?");
			break;
		}
	}
	return left;
}

static struct node *parse_stmt(struct parser *p);

static struct node *parse_if(struct parser *p)
{
	struct node *parent = node_create(2, NODE_IF);
	parent->children_num = 2;
	expect(p, TOKEN_IF);
	expect(p, TOKEN_LPAREN);
	parent->children[0] = parse_expr(p, 0);
	expect(p, TOKEN_RPAREN);
	parent->children[1] = parse_stmt(p);
	if (peek(p).type == TOKEN_ELSE) {
		parent->children_num = 3;
		parent->children = realloc(parent->children, parent->children_num * sizeof(*parent->children));
		advance(p);
		parent->children[2] = parse_stmt(p);
	}
	return parent;
}

static struct node *parse_while(struct parser *p)
{
	struct node *parent = node_create(2, NODE_WHILE);
	parent->children_num = 2;
	expect(p, TOKEN_WHILE);
	expect(p, TOKEN_LPAREN);
	parent->children[0] = parse_expr(p, 0);
	expect(p, TOKEN_RPAREN);
	parent->children[1] = parse_stmt(p);
	return parent;
}

static struct node *parse_switch(struct parser *p)
{
	struct node *parent = node_create(2, NODE_SWITCH);
	parent->children_num = 2;
	expect(p, TOKEN_SWITCH);
	expect(p, TOKEN_LPAREN);
	parent->children[0] = parse_expr(p, 0);
	expect(p, TOKEN_RPAREN);
	parent->children[1] = parse_stmt(p);
	return parent;
}

static struct node *parse_do(struct parser *p)
{
	struct node *parent = node_create(2, NODE_DO);
	parent->children_num = 2;
	expect(p, TOKEN_DO);
	parent->children[0] = parse_stmt(p);
	expect(p, TOKEN_WHILE);
	expect(p, TOKEN_LPAREN);
	parent->children[1] = parse_expr(p, 0);
	expect(p, TOKEN_RPAREN);
	expect(p, TOKEN_SEMI);
	return parent;
}

static struct node *parse_decl(struct parser *p);

static struct node *parse_for(struct parser *p)
{
	struct node *parent = node_create(4, NODE_FOR);
	parent->children_num = 4;
	expect(p, TOKEN_FOR);
	expect(p, TOKEN_LPAREN);
	sym_stack_push(&p->symtable);
	if (peek(p).type == TOKEN_INT || peek(p).type == TOKEN_BOOL) {
		parent->children[0] = parse_decl(p);
	} else {
		parent->children[0] = parse_expr(p, 0);
		expect(p, TOKEN_SEMI);
	}
	parent->children[1] = parse_expr(p, 0);
	expect(p, TOKEN_SEMI);
	parent->children[2] = parse_expr(p, 0);
	expect(p, TOKEN_RPAREN);
	parent->children[3] = parse_stmt(p);
	sym_stack_pop(&p->symtable);
	return parent;
}

static struct node *parse_brk(struct parser *p)
{
	expect(p, TOKEN_BRK);
	struct node *n = node_create(0, NODE_BRK);
	expect(p, TOKEN_SEMI);
	return n;
}

static struct node *parse_cont(struct parser *p)
{
	expect(p, TOKEN_CONT);
	struct node *n = node_create(0, NODE_CONT);
	expect(p, TOKEN_SEMI);
	return n;
}

static struct node *parse_ret(struct parser *p)
{
	expect(p, TOKEN_RET);
	if (peek(p).type == TOKEN_SEMI) {
		struct node *n = node_create(0, NODE_RET);
		n->d_type = DTYPE_VOID;
		advance(p);
		return n;
	}
	struct node *n = node_create(1, NODE_RET);
	n->children_num = 1;
	n->children[0] = parse_expr(p, 0);
	n->d_type = n->children[0]->d_type;
	expect(p, TOKEN_SEMI);
	return n;
}

/*
 * Per N3220:
 * (simplified) A declaration is a type keyword followed by a variable name and optionally followed by an equals sign and expression.
 * It is always terminated with a semicolon.
*/
static struct node *parse_decl(struct parser *p)
{
	enum token_type tt = peek(p).type;
	if (tt == TOKEN_INT || tt == TOKEN_BOOL) {
		advance(p);                                                                         
	} else {
		fprintf(stderr, "unrecognized declaration type of %s\n", peek(p).name);
		exit(1);
	}
	char var_name[32];
	strncpy(var_name, expect(p, TOKEN_ID).name, MAX_ID_LEN - 1);
	var_name[MAX_ID_LEN - 1] = '\0';
	enum data_type dt;
	if (tt == TOKEN_INT) {
		dt = DTYPE_INT;
	} else {
		dt = DTYPE_BOOL;
	}
	symtable_store(&p->symtable, var_name, dt);
	struct token t = advance(p);
	if (t.type == TOKEN_SEMI) {
		return NULL;
	} else if (t.type == TOKEN_ASSIGN) {
		struct node *parent = node_create(2, NODE_BIN);
		parent->children_num = 2;
		parent->d_type = dt;
		strcpy(parent->op, "=");
		parent->children[0] = node_create(0, NODE_VAR_NAME);
		strcpy(parent->children[0]->name, var_name);
		parent->children[0]->id = symtable_load(&p->symtable, var_name).id;
		parent->children[0]->d_type = dt;
		parent->children[1] = parse_expr(p, 0);
		expect(p, TOKEN_SEMI);
		return parent;
	} else {
		fprintf(stderr, "parsing error: unrecognized initializer\n");
		exit(1);
	}
}

/*
 * Per N3220:
 * An expression statement is either a null statement or an expression followed by a semicolon.
*/
static struct node *parse_expr_stmt(struct parser *p)
{
	if (peek(p).type == TOKEN_SEMI) {
		advance(p);
		return NULL;
	}
	struct node *n = parse_expr(p, 0);
	expect(p, TOKEN_SEMI);
	return n;
}

/*
 * Per N3220:
 * A selection statement is an if, an if/else, or a switch statement.
*/
static struct node *parse_sel_stmt(struct parser *p)
{
	switch (peek(p).type) {
	case TOKEN_IF:
		return parse_if(p);
	case TOKEN_SWITCH:
		return parse_switch(p);
	default:
		fprintf(stderr, "parsing error: unrecognized selection statement\n");
		exit(1);
	}
}

/*
 * Per N3220:
 * An iteration statement is a while, a do/while, or a for loop.
*/
static struct node *parse_iter_stmt(struct parser *p)
{
	switch(peek(p).type) {
	case TOKEN_WHILE:
		return parse_while(p);
	case TOKEN_DO:
		return parse_do(p);
	case TOKEN_FOR:
		return parse_for(p);
	default:
		fprintf(stderr, "parsing error: unrecognized iteration statement\n");
		exit(1);
	}
}

static struct node *parse_cmpnd_stmt(struct parser *p);

/*
 * Per N3220:
 * A primary block is a selection statement, an iteration statement, or a compound statement.
*/
static struct node *parse_prim_block(struct parser *p)
{
	switch (peek(p).type) {
	case TOKEN_IF:
	case TOKEN_SWITCH:
		return parse_sel_stmt(p);
	case TOKEN_WHILE:
	case TOKEN_DO:
	case TOKEN_FOR:
		return parse_iter_stmt(p);
	case TOKEN_LBRACE:
		return parse_cmpnd_stmt(p);
	default:
		fprintf(stderr, "parsing error: unrecognized primary block\n");
		exit(1);
	}
}

/*
 * Per N3220:
 * A primary block is a goto statement, a break statement, a continue statement, or a return statement.
 * GOTO NOT YET SUPPORTED
*/
static struct node *parse_jmp_stmt(struct parser *p)
{
	switch (peek(p).type) {
	case TOKEN_BRK:
		return parse_brk(p);
	case TOKEN_CONT:
		return parse_cont(p);
	case TOKEN_RET:
		return parse_ret(p);
	default:
		fprintf(stderr, "parsing error: unrecognized jump statement\n");
		exit(1);
	}
}

static struct node *parse_label(struct parser *p)
{
	struct node *n;
	switch (peek(p).type) {
	case TOKEN_CASE:
		n = node_create(1, NODE_CASE);
		n->children_num = 1;
		advance(p);
		n->children[0] = parse_expr(p, 0);
		expect(p, TOKEN_COL);
		return n;
	
	case TOKEN_DEFAULT:
		n = node_create(0, NODE_DEFAULT);
		advance(p);
		expect(p, TOKEN_COL);
		return n;
	
	default:
		fprintf(stderr, "invalid label\n");
		exit(1);
	}
}

static struct node *parse_labeled_stmt(struct parser *p)
{
	struct node *label = parse_label(p);
	label->children = realloc(label->children, ++label->children_num * sizeof(*label->children));
	label->children[label->children_num - 1] = parse_stmt(p);
	return label;
}

/*
 * Per N3220:
 * An unlabeled statement is a primary block, a jump statement, or an expression statement.
*/
static struct node *parse_unlabeled_stmt(struct parser *p)
{
	switch (peek(p).type) {
	case TOKEN_IF:
	case TOKEN_SWITCH:
	case TOKEN_WHILE:
	case TOKEN_DO:
	case TOKEN_FOR:
	case TOKEN_LBRACE:
		return parse_prim_block(p);
	case TOKEN_BRK:
	case TOKEN_CONT:
	case TOKEN_RET:
		return parse_jmp_stmt(p);
	default:
		return parse_expr_stmt(p);
	}
}

/*
 * Per N3220:
 * A statement is either a labeled statement or unlabeled statement.
*/
static struct node *parse_stmt(struct parser *p)
{
	switch (peek(p).type) {
	case TOKEN_CASE:
	case TOKEN_DEFAULT:
		return parse_labeled_stmt(p);
	default:
		return parse_unlabeled_stmt(p);
	}
}

/*
 * Per N3220:
 * A block item is a declaration, an unlabeled statement, or a label.
*/
static struct node *parse_block_item(struct parser *p)
{
	switch (peek(p).type) {
	case TOKEN_INT:
	case TOKEN_BOOL:
		return parse_decl(p);
	case TOKEN_CASE:
	case TOKEN_DEFAULT:
		return parse_label(p);
	default:
		return parse_unlabeled_stmt(p);
	}
}

/*
 * Per N3220:
 * A compound statement is a list of block items.
*/
static struct node *parse_cmpnd_stmt(struct parser *p)
{
	struct node *block = node_create(MAX_STMTS, NODE_BLOCK);
	expect(p, TOKEN_LBRACE);
	sym_stack_push(&p->symtable);
	for (int i = 0; peek(p).type != TOKEN_RBRACE; i++) {
		struct node *n = parse_block_item(p);
		if (!n) {
			i--;
		} else {
			block->children[i] = n;
			block->children_num++;
		}
	}
	sym_stack_pop(&p->symtable);
	advance(p);
	return block;
}

static struct node *parse_func_decl(struct parser *p)
{
	struct node *func = node_create(MAX_STMTS, NODE_FUNC);
	if (peek(p).type == TOKEN_VOID) {
		func->d_type = DTYPE_VOID;
		advance(p);
	} else if (peek(p).type == TOKEN_INT) {
		func->d_type = DTYPE_INT;
		advance(p);
	} else {
		fprintf(stderr, "invalid return type");
	}
	strcpy(func->name, expect(p, TOKEN_ID).name);
	expect(p, TOKEN_LPAREN);
	expect(p, TOKEN_VOID);
	expect(p, TOKEN_RPAREN);
	if (peek(p).type != TOKEN_LBRACE) {
		fprintf(stderr, "function %s must be followed by a block enclosed by {}\n", func->name);
		exit(1);
	}

	func->children_num = 1;
	func->children[0] = parse_cmpnd_stmt(p);
	return func;
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
	sym_stack_push(&p->symtable);
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
	case NODE_BLOCK:
		printf("BLOCK\n");
		break;
	case NODE_IF:
		printf("IF\n");
		break;
	case NODE_SWITCH:
		printf("SWITCH\n");
		break;
	case NODE_CASE:
		printf("CASE\n");
		break;
	case NODE_DEFAULT:
		printf("DEFAULT\n");
		break;
	case NODE_WHILE:
		printf("WHILE\n");
		break;
	case NODE_DO:
		printf("DO\n");
		break;
	case NODE_FOR:
		printf("FOR\n");
		break;
	case NODE_BRK:
		printf("BRK\n");
		break;
	case NODE_CONT:
		printf("CONT\n");
		break;
	case NODE_RET:
		printf("RET\n");
		break;
	case NODE_VAR_NAME:
		printf("%s VAR %s\n", (n->d_type == DTYPE_BOOL) ? "BOOL" : "INT", n->name);
		break;
	case NODE_INT_LIT:
		printf("INT %d\n", n->val);
		break;
	case NODE_UN:
		printf("UN %s\n", n->op);
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
