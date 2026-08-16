#ifndef LEXER_H
#define LEXER_H

#define MAX_ID_LEN 32
#define MAX_TOKENS 100

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
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_STAR,
	TOKEN_DIV,
	TOKEN_MOD,
	TOKEN_AMP,
	TOKEN_PIPE,
	TOKEN_XOR,
	TOKEN_LT,
	TOKEN_GT,
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

struct lexer *lexer_create_and_load(const char *filename, int prog_size);
void lexer_free(struct lexer *l);

struct token *lex_program(struct lexer *l);
void tokens_free(struct token *tokens);

void tokens_print(struct token *tokens);

#endif
