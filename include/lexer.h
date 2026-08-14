#ifndef LEXER_H
#define LEXER_H

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

struct token {
	enum token_type type;
	union {
		char name[MAX_ID_LEN];
		int val;
	};
};

extern const char *token_names[];

struct lexer *lexer_create_and_load(const char *filename, int prog_size);
void lexer_free(struct lexer *l);

struct token *next_token(struct lexer *l);

#endif
