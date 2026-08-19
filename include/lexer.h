#ifndef LEXER_H
#define LEXER_H

#define MAX_ID_LEN 32
#define MAX_TOKENS 100

enum token_type {
	TOKEN_ERR,

	// TYPES
	TOKEN_INT,
	TOKEN_BOOL,
	TOKEN_VOID,

	// SYMBOLS
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_LBRACE,
	TOKEN_RBRACE,
	TOKEN_SEMI,
	TOKEN_COL,

	// OPERATORS
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
	TOKEN_LE,
	TOKEN_GE,
	TOKEN_EQ,
	TOKEN_NE,
	TOKEN_BNOT,
	TOKEN_LNOT,

	// CONTROL FLOW
	TOKEN_IF,
	TOKEN_ELSE,
	TOKEN_WHILE,
	TOKEN_DO,
	TOKEN_FOR,
	TOKEN_BRK,
	TOKEN_CONT,
	TOKEN_SWITCH,
	TOKEN_CASE,
	TOKEN_DEFAULT,
	TOKEN_GOTO,
	TOKEN_RET,

	// MISC
	TOKEN_ASSIGN,
	TOKEN_ID,
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
