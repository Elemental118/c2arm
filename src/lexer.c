#include <stdio.h>

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

int main(void)
{
	struct token tokens[MAX_TOKENS];
	printf("Hello world\n");
}
