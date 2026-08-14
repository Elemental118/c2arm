#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "types.h"

struct parser {
	struct token *tokens;
	int pos;
};

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
