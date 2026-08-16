#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "irgen.h"
#include "parser.h"
#include "lexer.h"
#include "types.h"

#define MAX_FILE_LEN 1000
#define MAX_TOKENS 100

int main(int argc, char **argv)
{
	// INIT
	setvbuf(stdout, NULL, _IOLBF, 0);
	bool verbose = false;
	int file_i = argc - 1;
	if (argc < 2 || argc > 3) {
		fprintf(stderr, "usage: ./compiler <file>\n");
		exit(1);
	}
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-v")) {
			verbose = true;
		} else {
			file_i = i;
		}
	}

	// LEXING
	struct lexer *l = lexer_create_and_load(argv[file_i], MAX_FILE_LEN);
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

	if (verbose) {
		for (int i = 0; i < MAX_TOKENS; i ++) {
			printf("%s\n", token_names[tokens[i].type]);
			if (tokens[i].type == TOKEN_EOF || tokens[i].type == TOKEN_ERR) {
				break;
			}
		}
		printf("\n");
	}
	
	// PARSING
	struct parser *p = parser_create_and_load(tokens, MAX_TOKENS);
	struct node *ast = parse_program(p);
	parser_free(p);
	if (verbose) {
		ast_print(ast);
		printf("\n");
	}
	free(tokens);

	// IRGEN
	struct irgen *irg = irgen_create_and_load();
	struct instr *ir = irgen_prog(irg, ast);
	irgen_free(irg);
	if (verbose) {
		ir_print(ir);
		printf("\n");
	}
	ast_free(ast);

	// CODEGEN
	struct codegen *cg = codegen_create();
	codegen_prog(cg, ir);
	codegen_print(cg);
	codegen_free(cg);
	ir_free(ir);
}
