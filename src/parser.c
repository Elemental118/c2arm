#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "types.h"

struct parser {
	struct token *tokens;
	int pos;
};

static struct token peek(struct parser *p)
{
	return p->tokens[p->pos];
}

static struct token advance(struct parser *p)
{
	return p->tokens[p->pos++];
}

static void expect(struct parser *p, enum token_type t) {
	if (peek(p).type == t) {
		advance(p);
	} else {
		fprintf(stderr, "parsing error\n");
		exit(1);
	}
}

struct parser *parser_create_and_load(struct token *tokens, int token_num)
{
	struct parser *p = malloc(sizeof(*p));
	p->tokens = malloc(token_num * sizeof(*p->tokens));
	memcpy(p->tokens, tokens, token_num * sizeof(*p->tokens));
	return p;
}

void parser_free(struct parser *p)
{
	free(p->tokens);
	free(p);
}
