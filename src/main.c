#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "types.h"

#define MAX_FILE_LEN 1000
#define MAX_TOKENS 100

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: ./compiler <file>\n");
		exit(1);
	}

	struct lexer *l = lexer_create_and_load(argv[1], MAX_FILE_LEN);
	if (!l) {
		fprintf(stderr, "error: invalid file\n");
		exit(1);
	}

	struct token *tokens = malloc(MAX_TOKENS * sizeof(*tokens));
	for (int i = 0; i < MAX_TOKENS; i++) {
		tokens[i] = next_token(l);
		if (tokens[i].type == TOKEN_EOF || tokens[i].type == TOKEN_ERR) {
			break;
		}
	}

	for (int i = 0; i < MAX_TOKENS; i ++) {
		printf("%s\n", token_names[tokens[i].type]);
		if (tokens[i].type == TOKEN_EOF || tokens[i].type == TOKEN_ERR) {
			break;
		}
	}

	for (int i = 0; i < MAX_TOKENS; i++) {
		if (tokens[i].type == TOKEN_EOF || tokens[i].type == TOKEN_ERR) {
			break;
		}
	}

	free(tokens);
	lexer_free(l);
}
