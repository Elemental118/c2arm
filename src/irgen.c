#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "irgen.h"
#include "parser.h"
#include "types.h"

#define MAX_INSTRS 100

struct irgen {
	struct instr *instrs;
	int pos;
};

static void irgen_stmt(struct irgen *ir, struct node *n)
{
	if (ir->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	ir->instrs[ir->pos].i_type = INSTR_ASSIGN;
	ir->instrs[ir->pos].d_type = n->children[0]->d_type;
	strcpy(ir->instrs[ir->pos].dest_name, n->children[0]->name);
	ir->instrs[ir->pos].op1_kind = OPERAND_LITERAL;
	ir->instrs[ir->pos++].op1_val = n->children[1]->val;
}

static void irgen_func(struct irgen *ir, struct node *n)
{
	if (ir->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	ir->instrs[ir->pos].i_type = INSTR_FUNC_START;
	ir->instrs[ir->pos].d_type = n->d_type;
	strcpy(ir->instrs[ir->pos++].dest_name, n->name);
	for (int i = 0; i < n->children_num; i++) {
		irgen_stmt(ir, n->children[i]);
	}
	if (ir->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	ir->instrs[ir->pos++].i_type = INSTR_FUNC_END;
}

struct instr *irgen_prog(struct irgen *ir, struct node *ast)
{
	for (int i = 0; i < ast->children_num; i++) {
		irgen_func(ir, ast->children[i]);
	}
	if (ir->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	ir->instrs[ir->pos++].i_type = INSTR_EOF;
	struct instr *instrs = malloc(ir->pos * sizeof(*instrs));
	memcpy(instrs, ir->instrs, ir->pos * sizeof(*instrs));
	return instrs;

}

void instr_free(struct instr *instrs)
{
	free(instrs);
}

struct irgen *irgen_create_and_load(void)
{
	struct irgen *ir = calloc(1, sizeof(*ir));
	ir->instrs = malloc(MAX_INSTRS * sizeof(*ir->instrs));
	return ir;
}

void irgen_free(struct irgen *ir)
{
	free(ir->instrs);
	free(ir);
}
