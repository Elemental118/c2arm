#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

#define MAX_FILE_LEN 1000
#define MAX_TOKENS 100
#define MAX_ID_LEN 32

enum token_type {
	TOKEN_ERR,

	TOKEN_INT,
	TOKEN_VOID,
	TOKEN_ID,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_LBRACE,
	TOKEN_RBRACE,
	TOKEN_SEMI,
	TOKEN_ASSIGN,
	TOKEN_INT_LIT,
	TOKEN_EOF
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

struct token {
	enum token_type type;
	union {
		char name[MAX_ID_LEN];
		int val;
	};
};

struct lexer {
	char *prog;
	int pos;
};

bool is_in_id(u8 c)
{
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

struct token *next_token(struct lexer *l)
{
	struct token *t = malloc(sizeof(*t));
	while (l->prog[l->pos] == ' ' || l->prog[l->pos] == '\n' || l->prog[l->pos] == '\r' || l->prog[l->pos] == '\t') {
		l->pos++;
	}

	char c = l->prog[l->pos];
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

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: ./compiler <file>\n");
		exit(1);
	}

	// Read file
	struct lexer *l = calloc(sizeof(struct lexer), 1);
	l->prog = calloc(MAX_FILE_LEN, sizeof(*l->prog));

	FILE *f;
	if (!(f = fopen(argv[1], "r"))) {
		fprintf(stderr, "error: invalid file\n");
		exit(1);
	};

	fread(l->prog, sizeof(u8), MAX_FILE_LEN, f);
	fclose(f);

	struct token *tokens = malloc(MAX_TOKENS * sizeof(*tokens));
	for (int i = 0; ; i++) {
		tokens[i] = *next_token(l);
		if (tokens[i].type == TOKEN_EOF || tokens[i].type == TOKEN_ERR) {
			break;
		}
	}

	printf("%s\n", l->prog);
	printf("\n\n\n");
	for (int i = 0; ;i ++) {
		printf("%s\n", token_names[tokens[i].type]);
		if (tokens[i].type == TOKEN_EOF || tokens[i].type == TOKEN_ERR) {
			break;
		}
	}
	printf("\n\n");

	free(tokens);
	free(l->prog);
	free(l);
}
