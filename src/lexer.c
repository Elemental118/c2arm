#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "types.h"

struct lexer {
	u8 *prog;
	int pos;
};

static const char *token_names[] = {
	[TOKEN_ERR] = "ERR",
	[TOKEN_INT] = "INT",
	[TOKEN_BOOL] = "BOOL",
	[TOKEN_VOID] = "VOID",
	[TOKEN_LPAREN] = "LPAREN",
	[TOKEN_RPAREN] = "RPAREN",
	[TOKEN_LBRACE] = "LBRACE",
	[TOKEN_RBRACE] = "RBRACE",
	[TOKEN_SEMI] = "SEMI",
	[TOKEN_COL] = "COL",
	[TOKEN_PLUS] = "PLUS",
	[TOKEN_MINUS] = "MINUS",
	[TOKEN_STAR] = "STAR",
	[TOKEN_DIV] = "DIV",
	[TOKEN_MOD] = "MOD",
	[TOKEN_AMP] = "AMP",
	[TOKEN_PIPE] = "PIPE",
	[TOKEN_XOR] = "XOR",
	[TOKEN_LT] = "LT",
	[TOKEN_GT] = "GT",
	[TOKEN_LE] = "LE",
	[TOKEN_GE] = "GE",
	[TOKEN_EQ] = "EQ",
	[TOKEN_NE] = "NE",
	[TOKEN_BNOT] = "BNOT",
	[TOKEN_LNOT] = "LNOT",
	[TOKEN_IF] = "IF",
	[TOKEN_ELSE] = "ELSE",
	[TOKEN_WHILE] = "WHILE",
	[TOKEN_DO] = "DO",
	[TOKEN_FOR] = "FOR",
	[TOKEN_BRK] = "BRK",
	[TOKEN_CONT] = "CONT",
	[TOKEN_SWITCH] = "SWITCH",
	[TOKEN_CASE] = "CASE",
	[TOKEN_DEFAULT] = "DEFAULT",
	[TOKEN_GOTO] = "GOTO",
	[TOKEN_RET] = "RET",
	[TOKEN_ASSIGN] = "ASSIGN",
	[TOKEN_ID] = "ID",
	[TOKEN_INT_LIT] = "INT_LIT",
	[TOKEN_EOF] = "EOF"
};

static bool is_in_id(u8 c)
{
	return isalpha(c) || (c == '_');
}

struct lexer *lexer_create_and_load(const char *filename, int prog_size)
{
	struct lexer *l = calloc(1, sizeof(*l));
	l->prog = calloc(prog_size + 1, sizeof(*l->prog));
	FILE *f;
	if (!(f = fopen(filename, "r"))) {
		free(l);
		free(l->prog);
		return NULL;
	};

	fread(l->prog, sizeof(u8), prog_size, f);
	fclose(f);
	return l;
}

void lexer_free(struct lexer *l)
{
	free(l->prog);
	free(l);
}

struct token next_token(struct lexer *l)
{
	struct token t;
	while (l->prog[l->pos] == ' ' || l->prog[l->pos] == '\n' || l->prog[l->pos] == '\r' || l->prog[l->pos] == '\t') {
		l->pos++;
	}

	u8 c = l->prog[l->pos];
	switch (c) {
	case '(':
		t.type = TOKEN_LPAREN;
		break;
	
	case ')':
		t.type = TOKEN_RPAREN;
		break;
	
	case '{':
		t.type = TOKEN_LBRACE;
		break;
	
	case '}':
		t.type = TOKEN_RBRACE;
		break;
	
	case '+':
		t.type = TOKEN_PLUS;
		break;
	
	case '-':
		t.type = TOKEN_MINUS;
		break;
	
	case '*':
		t.type = TOKEN_STAR;
		break;
	
	case '/':
		t.type = TOKEN_DIV;
		break;
	
	case '%':
		t.type = TOKEN_MOD;
		break;
	
	case '&':
		t.type = TOKEN_AMP;
		break;
	
	case '|':
		t.type = TOKEN_PIPE;
		break;
	
	case '^':
		t.type = TOKEN_XOR;
		break;
	
	case '<':
		if (l->prog[l->pos + 1] == '=') {
			l->pos++;
			t.type = TOKEN_LE;
		} else {
			t.type = TOKEN_LT;
		}
		break;
	
	case '>':
		if (l->prog[l->pos + 1] == '=') {
			l->pos++;
			t.type = TOKEN_GE;
		} else {
			t.type = TOKEN_GT;
		}
		break;
	
	case ';':
		t.type = TOKEN_SEMI;
		break;
	
	case ':':
		t.type = TOKEN_COL;
		break;
	
	case '=':
		if (l->prog[l->pos + 1] == '=') {
			l->pos++;
			t.type = TOKEN_EQ;
		} else {
			t.type = TOKEN_ASSIGN;
		}
		break;
	
	case '!':
		if (l->prog[l->pos + 1] == '=') {
			l->pos++;
			t.type = TOKEN_NE;
		} else {
			t.type = TOKEN_LNOT;
		}
		break;
	
	case '~':
		t.type = TOKEN_BNOT;
		break;
	
	case '\0':
		t.type = TOKEN_EOF;
		break;
	
	default:
		// INETGER LITERALS
		int original = l->pos;
		t.val = 0;
		t.type = TOKEN_INT_LIT;
		while ('0' <= c && c <= '9') {
			if (l->pos - original == MAX_ID_LEN - 1) {
				t.type = TOKEN_ERR;
				return t;
			}
			if (t.val > (INT_MAX - (c - '0')) / 10) {
				t.type = TOKEN_ERR;
				return t;
			}
			t.val *= 10;
			t.val += c - '0';
			c = l->prog[++l->pos];
		}
		if (original != l->pos) {
			l->pos--;
			break;
		}

		// IDENTIFIERS/KEYWORDS
		while (is_in_id(c)) {
			if (l->pos - original == MAX_ID_LEN - 1) {
				t.type = TOKEN_ERR;
				return t;
			}
			t.name[l->pos++ - original] = c;
			c = l->prog[l->pos];
		}
		if (original == l->pos) {
			t.type = TOKEN_ERR;
			break;
		}
		t.name[l->pos-- - original] = '\0';
		if (!strcmp(t.name, "int")) {
			t.type = TOKEN_INT;
		} else if (!strcmp(t.name, "bool")) {
			t.type = TOKEN_BOOL;
		} else if (!strcmp(t.name, "void")) {
			t.type = TOKEN_VOID;
		} else if (!strcmp(t.name, "if")) {
			t.type = TOKEN_IF;
		} else if (!strcmp(t.name, "else")) {
			t.type = TOKEN_ELSE;
		} else if (!strcmp(t.name, "while")) {
			t.type = TOKEN_WHILE;
		} else if (!strcmp(t.name, "do")) {
			t.type = TOKEN_DO;
		} else if (!strcmp(t.name, "for")) {
			t.type = TOKEN_FOR;
		} else if (!strcmp(t.name, "switch")) {
			t.type = TOKEN_SWITCH;
		} else if (!strcmp(t.name, "case")) {
			t.type = TOKEN_CASE;
		} else if (!strcmp(t.name, "break")) {
			t.type = TOKEN_BRK;
		} else if (!strcmp(t.name, "continue")) {
			t.type = TOKEN_CONT;
		} else if (!strcmp(t.name, "default")) {
			t.type = TOKEN_DEFAULT;
		} else if (!strcmp(t.name, "goto")) {
			t.type = TOKEN_GOTO;
		} else if (!strcmp(t.name, "return")) {
			t.type = TOKEN_RET;
		} else {
			t.type = TOKEN_ID;
		}
	}

	l->pos++;
	return t;
}

struct token *lex_program(struct lexer *l)
{
	struct token *tokens = malloc(MAX_TOKENS * sizeof(*tokens));
	for (int i = 0; i < MAX_TOKENS; i++) {
		tokens[i] = next_token(l);
		if (tokens[i].type == TOKEN_EOF || tokens[i].type == TOKEN_ERR) {
			break;
		}
	}
	return tokens;
}

void tokens_free(struct token *tokens)
{
	free(tokens);
}

void tokens_print(struct token *tokens)
{
	for (int i = 0; i < MAX_TOKENS; i ++) {
		printf("%s\n", token_names[tokens[i].type]);
		if (tokens[i].type == TOKEN_EOF || tokens[i].type == TOKEN_ERR) {
			break;
		}
	}
}
