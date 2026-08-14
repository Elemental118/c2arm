#include <stdio.h>
#include <stdlib.h>

#include "types.h"

#define MAX_FILE_LEN 1000
#define MAX_TOKENS 100

enum token_type {
	TOKEN_NONE,

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

struct token {
	enum token_type type;
	union {
		char name[32];
		int val;
	};
};

struct lexer {
	char *prog;
	int pos;
};

struct token next_token(struct lexer *l)
{
	// Stuff to make it compile
	struct token t;
	t.type = TOKEN_NONE;
	t.val = l->pos;
	return t;
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: ./compiler <file>\n");
		exit(1);
	}

	// Read file
	struct lexer *l = malloc(sizeof(struct lexer));
	l->prog = calloc(MAX_FILE_LEN, sizeof(*l->prog));

	FILE *f;
	if (!(f = fopen(argv[1], "r"))) {
		fprintf(stderr, "error: invalid file\n");
		exit(1);
	};

	fread(l->prog, sizeof(u8), MAX_FILE_LEN, f);
	fclose(f);

	// struct token tokens[MAX_TOKENS];

	printf("%s\n", l->prog);
	free(l->prog);
	free(l);
}
