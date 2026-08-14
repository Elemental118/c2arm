#include <stdio.h>
#include <stdlib.h>

#include "types.h"

#define MAX_FILE_LEN 1000
#define MAX_TOKENS 100

enum token_type {
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

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: ./compiler <file>\n");
		exit(1);
	}

	// Read file
	u8 prog[MAX_FILE_LEN];
	FILE *f;
	if (!(f = fopen(argv[1], "r"))) {
		fprintf(stderr, "error: invalid file\n");
		exit(1);
	};
	fread(prog, sizeof(u8), MAX_FILE_LEN, f);
	fclose(f);

	// struct token tokens[MAX_TOKENS];

	printf("%s\n", prog);
}
