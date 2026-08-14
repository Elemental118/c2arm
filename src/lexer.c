#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "types.h"

struct lexer {
	u8 *prog;
	int pos;
};

const char *token_names[] = {
    [TOKEN_ERR] = "TOKEN_ERR",
    [TOKEN_INT] = "TOKEN_INT",
    [TOKEN_VOID] = "TOKEN_VOID",
    [TOKEN_ID] = "TOKEN_ID",
    [TOKEN_LPAREN] = "TOKEN_LPAREN",
    [TOKEN_RPAREN] = "TOKEN_RPAREN",
    [TOKEN_LBRACE] = "TOKEN_LBRACE",
    [TOKEN_RBRACE] = "TOKEN_RBRACE",
    [TOKEN_SEMI] = "TOKEN_SEMI",
    [TOKEN_ASSIGN] = "TOKEN_ASSIGN",
    [TOKEN_INT_LIT] = "TOKEN_INT_LIT",
    [TOKEN_EOF] = "TOKEN_EOF"
};

static bool is_in_id(u8 c)
{
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

struct lexer *lexer_create_and_load(const char *filename, int prog_size) {
	struct lexer *l = calloc(sizeof(struct lexer), 1);
	l->prog = calloc(prog_size, sizeof(*l->prog));
	FILE *f;
	if (!(f = fopen(filename, "r"))) {
		return NULL;
	};

	fread(l->prog, sizeof(u8), prog_size, f);
	fclose(f);
	return l;
}

void lexer_free(struct lexer *l) {
	free(l->prog);
	free(l);
}

struct token *next_token(struct lexer *l)
{
	struct token *t = malloc(sizeof(*t));
	while (l->prog[l->pos] == ' ' || l->prog[l->pos] == '\n' || l->prog[l->pos] == '\r' || l->prog[l->pos] == '\t') {
		l->pos++;
	}

	u8 c = l->prog[l->pos];
	switch (c) {
	case '(':
		t->type = TOKEN_LPAREN;
		break;
	
	case ')':
		t->type = TOKEN_RPAREN;
		break;
	
	case '{':
		t->type = TOKEN_LBRACE;
		break;
	
	case '}':
		t->type = TOKEN_RBRACE;
		break;
	
	case ';':
		t->type = TOKEN_SEMI;
		break;
	
	case '=':
		t->type = TOKEN_ASSIGN;
		break;
	
	case '\0':
		t->type = TOKEN_EOF;
		break;
	
	default:
		if ('0' <= c && c <= '9') {
			t->type = TOKEN_INT_LIT;
			t->val = c - '0';
			break;
		}
		int original = l->pos;
		while (is_in_id(c)) {
			t->name[l->pos++ - original] = c;
			c = l->prog[l->pos];
		}
		if (original == l->pos) {
			t->type = TOKEN_ERR;
			break;
		}
		t->name[l->pos-- - original] = '\0';
		if (!strcmp(t->name, "int")) {
			t->type = TOKEN_INT;
		} else if (!strcmp(t->name, "void")) {
			t->type = TOKEN_VOID;
		} else {
			t->type = TOKEN_ID;
		}
	}

	l->pos++;
	return t;
}
